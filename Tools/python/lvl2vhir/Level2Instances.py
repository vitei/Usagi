import sys, os, math
import LevelEditor
import yaml

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append( root + '/lvlUtil' )
import EntityYaml

def makeYamlTreeBase():
    blankData = {}
    blankData['Billboard'] = False
    blankData['Handedness'] = 'left'
    blankData['Instance'] = True
    blankData['Format'] = 'transform'
    return blankData

class InstanceObject:
    def __init__(self, gobj):
        self.gobj = gobj
        self.groupNo = -1 # Invalidate

class InstancesConverter:
    def __init__(self, distance):
        self.hash = {}
        self.distance = distance

    def convert(self, lvlPath):
        lvl = LevelEditor.Game.Level( lvlPath )

        instancesFolder = None

        # Get all top-level folders, except for the sublevels folder if
        # present.
        folders = lvl.getFolders(['SubLevels'])
        for folder in folders:
            if folder.attrib['name'] == 'Instances':
                instancesFolder = folder
                break

        # Instances folder is found
        if not instancesFolder is None:
            self.gatherObjectsRecursively( instancesFolder )

        if len( self.hash ) == 0:
            blankData = makeYamlTreeBase()
            blankData['ModelName'] = 'dummy'
            blankData['Length'] = 0
            blankData['Nodes'] = []
            return [ blankData ]

        for key, values in list(self.hash.items()):
            self.groupInstances( values )

        yamlTree = []
        for key, values in list(self.hash.items()):
            yamlTree.extend( self.createInstancesYaml( key, values ) )

        return yamlTree

    def gatherObjectsRecursively(self, folder):
        for obj in folder:
            # child folder?
            if obj.tag == "{gap}folder":
                self.gatherObjectsRecursively( obj )
                continue

            gobj = LevelEditor.Game.GameObject( obj )
            cmdlPath = EntityYaml.searchModelComponent( gobj.getURI() )
            cmdlPath = cmdlPath.replace( 'Models/', '' ) + '.vmdc'
            if cmdlPath not in self.hash:
                self.hash[cmdlPath] = {}

            self.hash[cmdlPath][gobj.getName()] = InstanceObject(gobj)#obj

    def groupInstances( self, objs ):
        indexA = 0
        instances = list(objs.items())
        for keyA, insObjA in instances:

            # Set group number initially
            if insObjA.groupNo == -1:
                insObjA.groupNo = indexA

            if indexA == len( instances ) - 1:
                break

            posA = insObjA.gobj.getTranslation()

            indexB = indexA + 1
            while indexB < len( instances ):
                distance = LevelEditor.Game.calcDistance( posA, instances[indexB][1].gobj.getTranslation() )
                if distance <= self.distance:
                    instances[indexB][1].groupNo = insObjA.groupNo
                indexB += 1

            indexA += 1

    def createInstancesYaml( self, modelPath, objs ):
        groupNoNum = len( objs )
        yamlTreeList = []

        for groupNo in range(groupNoNum):
            nodes = []
            #print modelPath
            for key, insObj in list(objs.items()):
                if not insObj.groupNo == groupNo:
                    continue

                node = {}
            
                node['name'] = os.path.basename( insObj.gobj.getURI() )

                # Setup transformation
                trans = insObj.gobj.getTranslation()
                rot = insObj.gobj.getRotate()
                scale = insObj.gobj.getScale()

                # Reinterpret as Left-handed coordinate
                trans = LevelEditor.Game.convertTranslationRHtoLH( trans )
                rot = LevelEditor.Game.convertRotationRHtoLH( rot )

                rad2deg = 180 / math.pi
                node['rotation'] = [ rot.x * rad2deg, rot.y * rad2deg, rot.z * rad2deg ]
                node['scale'] = [ scale.x, scale.y, scale.z ]
                node['translation'] = [ trans.x, trans.y, trans.z ]
                #node['groupNo'] = insObj.groupNo # Debug info

                nodes.append( node )

            if len(nodes) > 0:
                yamlTree = makeYamlTreeBase()
                yamlTree['ModelName'] = modelPath
                yamlTree['Length'] = len( nodes )
                yamlTree['Nodes'] = nodes
                yamlTreeList.append( yamlTree )

        return yamlTreeList