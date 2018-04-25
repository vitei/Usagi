import sys, os, os.path, subprocess
from optparse import Option, OptionParser

import LevelEditor
import Level2Yaml
import Level2Instances
import yaml
import re

# Usagi nanopb directory
usagi_dir = os.getenv( 'USAGI_DIR' )
sys.path.append( usagi_dir + '/Engine/ThirdParty/nanopb/generator/proto' )

# Parent directory
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Original option processor
class MultipleOption(Option):
    ACTIONS = Option.ACTIONS + ("extend",)
    STORE_ACTIONS = Option.STORE_ACTIONS + ("extend",)
    TYPED_ACTIONS = Option.TYPED_ACTIONS + ("extend",)
    ALWAYS_TYPED_ACTIONS = Option.ALWAYS_TYPED_ACTIONS + ("extend",)

    def take_action(self, action, dest, opt, value, values, parser):
        if action == "extend":
            values.ensure_value(dest, []).append(value)
        else:
            Option.take_action(self, action, dest, opt, value, values, parser)

# Option parser
optparser = OptionParser(option_class=MultipleOption)
optparser.add_option("--dist", dest="distance", type="float")

(options, args) = optparser.parse_args()

def saveYaml( path, yamlData ):
    dumpedYaml = yaml.dump( yamlData )

    dumpedYaml = dumpedYaml.replace( "'<%=", "<%=" )
    dumpedYaml = dumpedYaml.replace( "%>'", "%>" )

    f = open( path, 'w' )
    f.write( dumpedYaml )
    f.close()

def processLevelXml( lvl, outputPath, targetDirs, format ):
    # sys.stderr.write("level does not have sublevels.\n")
    yamlTree = Level2Yaml.convertLevel( lvl, targetDirs, format )

    root = [ yamlTree ]

    if format == 'hierarchy':
        root = yamlTree

    saveYaml( outputPath, root )

def processLevelXmlSubLevels( lvl, sublvl, outputPath, targetDirs, format ):
    yamlTree = Level2Yaml.convertLevel( lvl, targetDirs, format )
    sublvl_yamlTree = Level2Yaml.convert( sublvl, targetDirs, format )

    if isinstance(yamlTree, dict):
        yamlTree['Nodes'].extend(sublvl_yamlTree['Nodes'])
        yamlTree['Length'] = len(yamlTree['Nodes'])
    else:
        yamlTree.extend(sublvl_yamlTree)

    root = [ yamlTree ]

    if format == 'hierarchy':
        root = yamlTree

    saveYaml( outputPath, root )

def main( args ):
    if len( args ) == 0:
        sys.stderr.write('ERROR: Need input path.\n')
        return
    inputPath = args[0]

    if not os.path.isfile(inputPath):
        sys.stderr.write("ERROR: Specified level file doesn't exist.\n")
        return
    outputPath = args[1]

    outputPath_positions = args[2]
    outputPath_instances = args[3]

    outputPathWithoutExt = os.path.splitext(outputPath)[0]    

    # Suffixes.
    suffixPosition = '_pos.yml'
    suffixInstances = '_inst.yml'

    # Load the level file.
    lvl = LevelEditor.Game.Level( inputPath )

    # Do we have sublevels?
    hasSublevels = 'sublevels' in [f.attrib['name'].lower() for f in lvl.getFolders()]
    if hasSublevels:
        outputPathTokens = os.path.splitext(outputPath)

        # The build step will ask us for a particular sublevel
        # where given a input level:
        #
        #   colosseumMulti.lvl
        #
        # the designated sublevel needs to be extracted from the outputPath
        # given as:
        #
        #   colosseumMulti_forEscape.yml

        # Identify the name of the input level.
        inputBasename = os.path.splitext(os.path.basename(inputPath))[0]

        # Use the name of the input level to extract the name of the
        # sublevel. Early out if we can't do this.
        sublevelRegexMatches = re.compile(inputBasename + "_(.*).yml").match(os.path.basename(outputPath))
        if len(sublevelRegexMatches.groups()) != 1:
            sys.stderr.write("ERROR: Couldn't unpack sublevel name from requested output filenames (should be of form, levelname_sublevelname.yml).\n")
            return
        sublevelName = sublevelRegexMatches.group(1)    

        # Convert instances.
        instConverter = Level2Instances.InstancesConverter( options.distance )
        instances_yamlTree = instConverter.convert( inputPath )

        # Convert sublevels.
        sublevelsRoot = [f for f in lvl.gameObjectFolder.findall( './gap:folder' ) if f.attrib['name'].lower() == 'sublevels'][0]
        sublevel = sublevelsRoot.find( './gap:folder[@name="' + sublevelName + '"]', { 'gap' : 'gap' } )

        # Output hierarchy.
        targetDirs = ( 'Props', 'Bases', 'Instances' )
        processLevelXmlSubLevels( lvl, sublevel, outputPath, targetDirs, 'hierarchy' )

        # Output positions.
        targetDirs = ( 'Items', 'SpawnPoints', 'AI', 'CameraControlPoints' )
        processLevelXmlSubLevels( lvl, sublevel, outputPath_positions, targetDirs, 'positions' )

        # Output instances.
        if not instances_yamlTree is None:
            saveYaml( outputPath_instances, instances_yamlTree )
    else:
        # Output hierarchy. Bases folder is handled as a props folder.
        targetDirs = ( 'Props', 'Bases', 'Instances' )
        processLevelXml( lvl, outputPath, targetDirs, 'hierarchy' )

        # Output positions.
        targetDirs = ( 'Items', 'SpawnPoints', 'AI', 'CameraControlPoints' )
        processLevelXml( lvl, outputPath_positions, targetDirs, 'positions' )

        # Convert instances.
        instConverter = Level2Instances.InstancesConverter( options.distance )
        yamlTree = instConverter.convert( inputPath )
        if not yamlTree is None:
            saveYaml( outputPath_instances, yamlTree )

if __name__ == "__main__":
    main(args)
