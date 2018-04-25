import os, sys
import xml.etree.ElementTree as etree

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append( root + '/lvl2vhir' )
sys.path.append( root + '/lvlUtil' )

import LevelEditor
import EntityYaml
import FileIO


AI_BLOCKING_AREA_SUFFIX = '_AIBlockingArea'

#
# Model bounding box representation
class BoundingBox:
    def __init__(self, uri):
        self.uri = uri
        cmdlPath = EntityYaml.searchModelComponent( uri )
        self.scale = LevelEditor.Game.Vector3( 1.0, 1.0, 1.0 )
        self.setup( cmdlPath )

    def setup( self, cmdlPath ):
        print cmdlPath
        usagi_dir = os.getenv( 'USAGI_DIR' )

        fullPath = usagi_dir + '/Data/' + cmdlPath + '.cmdl'
        xmlDoc = etree.parse( open( fullPath, 'r' ) )
        root = xmlDoc.getroot()

        # get all <OrientedBoundingBox> tags from whole XML
        OBBs = root.findall( './/OrientedBoundingBox' )

        minmax = [ [None, None], [None, None] ] # only xz plane is considered. minmax[x,z][min,max]
        for obb in OBBs:
            #print obb.tag
            self.processOBB( obb, minmax )

        centerX = ( minmax[0][0] + minmax[0][1] ) / 2.0
        centerZ = ( minmax[1][0] + minmax[1][1] ) / 2.0
        self.scale.x = minmax[0][1] - minmax[0][0]
        self.scale.z = minmax[1][1] - minmax[1][0]
        self.scale.y = 2.0 # Set twice large for emphasizing

    def processOBB( self, obb, minmax ):
        # Calc min and max on each axis
        centerX = 0
        centerZ = 0
        sizeX = 0
        sizeZ = 0
        for child in obb:
            if child.tag == 'CenterPosition':
                centerX = float( child.attrib['X'] )
                centerZ = float( child.attrib['Z'] )
            elif child.tag == 'Size':
                sizeX = float( child.attrib['X'] )
                sizeZ = float( child.attrib['Z'] )
        
        minX = centerX - ( sizeX / 2 )
        maxX = centerX + ( sizeX / 2 )
        minZ = centerZ - ( sizeZ / 2 )
        maxZ = centerZ + ( sizeZ / 2 )

        # Update
        if minmax[0][0] is None:
            # Initial update
            minmax[0][0] = minX # x-min
            minmax[0][1] = maxX # x-max
            minmax[1][0] = minZ # z-min
            minmax[1][1] = maxZ # z-max
        else:
            minmax[0][0] = min( minmax[0][0], minX ) # x-min
            minmax[0][1] = max( minmax[0][1], maxX ) # x-max
            minmax[1][0] = min( minmax[1][0], minZ ) # z-min
            minmax[1][1] = max( minmax[1][1], maxZ ) # z-max

class AIBlockingArea:
    def __init__(self, bb, trans, rot):
        self.bb = bb
        self.trans = trans
        self.rot = rot
#
# AI blocking area
class AIBlockingAreaGenerator:
    def __init__(self):
        self.cache = {}
        self.areas = {}

    def createAIBlockingAreas( self, folder, lvl ):
        for obj in folder:
            # Sub folder found
            if obj.tag == "{gap}folder":
                self.createAIBlockingAreas( obj, lvl )
                continue

            gobj = LevelEditor.Game.GameObject( obj )
            uniqueName = gobj.getName() + AI_BLOCKING_AREA_SUFFIX

            # If it already exists. Skip.
            if not lvl.searchGameObject( uniqueName ) is None:
                continue

            uri = gobj.getURI()
            if not uri in self.cache:
                self.cache[uri] = BoundingBox( uri )

            trans = gobj.getTranslation()
            rot = gobj.getRotate()
            self.areas[uniqueName] = AIBlockingArea( self.cache[uri], trans, rot )

    def injectNewAreas(self, aiFolder):
        for key, area in self.areas.items():
            newElem = etree.Element( '{gap}gameObject' )

            gobj = LevelEditor.Game.GameObject( newElem )
            gobj.setType( "aiBlockingAreaType" )
            gobj.setAttribute( "name", key ) # Unique name
            
            # Transform
            rotationMtxX = LevelEditor.Game.Matrix33()
            rotationMtxX.createRotationX( area.rot.x )
            rotationMtxY = LevelEditor.Game.Matrix33()
            rotationMtxY.createRotationY( area.rot.y )
            rotationMtxZ = LevelEditor.Game.Matrix33()
            rotationMtxZ.createRotationZ( area.rot.z )

            scaleMtx = LevelEditor.Game.Matrix33()
            scale = area.bb.scale
            scaleMtx.createScaleMatrix( scale )

            #mtx = rotationMtx
            #mtx.multiply( scaleMtx )
            mtx = LevelEditor.Game.Matrix33()
            mtx.multiply( scaleMtx )
            mtx.multiply( rotationMtxX )
            mtx.multiply( rotationMtxY )
            mtx.multiply( rotationMtxZ )
            gobj.setTransform( mtx, area.trans )
            
            gobj.setTranslation( area.trans )
            gobj.setRotate( area.rot )
            gobj.setScale( scale )

            # Default parameters
            gobj.setAttribute( "pivot", "0 0 0" )
            gobj.setAttribute( "visible", "true" )
            gobj.setAttribute( "locked", "false" )
            gobj.setAttribute( "transformationType", "15" )
            gobj.setAttribute( "color", "-1" )
            gobj.setAttribute( "emissive", "0" )
            gobj.setAttribute( "specular", "0" )
            gobj.setAttribute( "specularPower", "1" )
            gobj.setAttribute( "diffuse", "" )
            gobj.setAttribute( "normal", "" )
            gobj.setAttribute( "textureTransform", "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1" )

            aiFolder.append( newElem )

def arrangeForAI( lvl ):
    targetDirs = ( 'Props', 'Bases', 'Instances' )

    areaGen = AIBlockingAreaGenerator()

    aiFolder = None
    folders = lvl.getFolders()
    for folder in folders:
        folderName = folder.attrib['name']
        if folderName in targetDirs:
            # Process this folder
            areaGen.createAIBlockingAreas( folder, lvl )
        elif folderName == 'AI':
            aiFolder = folder

    if aiFolder is None:
        print 'ERROR: AI folder not found!!'
        return False

    areaGen.injectNewAreas( aiFolder )
    return True

def main( argv ):
    openPath = argv[1]
    lvl = LevelEditor.Game.Level( openPath )
    
    if arrangeForAI( lvl ):
        FileIO.saveLevelXml( openPath, lvl )

if __name__ == "__main__":
    main( sys.argv )