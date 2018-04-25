import os, sys
import yaml

class EntityYaml:
    def __init__(self, yamlPath):
        yamlData = self.loadYaml(yamlPath)


        self.inherits = ''
        self.modelName = ''

        # First layer should be a list of Components
        self.traverseComponents( yamlData )

        #f.close()

    def loadYaml(self, yamlPath):
        usagi_dir = os.getenv( 'USAGI_DIR' )
        f = open( usagi_dir + '/Data/' + yamlPath, 'r' )

        yamlData = yaml.load(f)
        f.close()
        return yamlData

    def traverseComponents(self, yamlData):
        self.compoName = ''
        for key, value in yamlData.items():
            self.compoName = key

            if not value is None:
                self.traverseComponentVariables(value)

    def traverseComponentVariables(self, vals):
        if self.compoName == 'Inherits':
            self.inherits = vals[0] # 'vals' should be a list instance.
        else:
            if (type(vals) is dict):
                for valKey, valValue in vals.iteritems():
                    if not isinstance( valValue, dict):
                        self.catchValue( valKey, valValue )
                    else:
                        pass
            elif (type(vals) is list):
                for listItem in vals:
                    for valKey, valValue in listItem.iteritems():
                        if not isinstance( valValue, dict):
                            self.catchValue( valKey, valValue )
                        else:
                            pass

    def catchValue( self, valKey, valValue ):
        if self.compoName == 'ModelComponent' and valKey == 'name':
            self.modelName = valValue

##
# Search ModelComponent's name
def searchModelComponent( yamlPath ):
    cmdlPath = ''
    while not cmdlPath:
        entity = EntityYaml( yamlPath )
        if entity.modelName:
            cmdlPath = 'Models/' + entity.modelName
            cmdlPath, ext = os.path.splitext( cmdlPath )
            break;
        elif not entity.inherits:
            break; # No inheritance anymore

        yamlPath = 'Entities/' + entity.inherits + '.yml'

    return cmdlPath