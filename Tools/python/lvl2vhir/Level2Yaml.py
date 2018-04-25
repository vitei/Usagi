import sys, os, string, math
import LevelEditor
import yaml
import zlib

INST_SUFFIX = '_inst'

def createInstanceEntity( yamlPath, ext ):
    usagi_dir = os.getenv( 'USAGI_DIR' )
    f = open( usagi_dir + '/Data/' + yamlPath + ext, 'r' )
    yamlData = yaml.load(f)
    f.close

    if yamlData.has_key('ModelComponent'):
        yamlData.pop('ModelComponent')

    return yamlData

def makeEntity( obj, format, isInst ):
    enti = {}

    gobj = LevelEditor.Game.GameObject(obj)
    trans = gobj.getTranslation()
    rot = gobj.getRotate()
    scale = gobj.getScale()

    # Reinterpret as Left-handed coordinate
    trans = LevelEditor.Game.convertTranslationRHtoLH( trans )
    rot = LevelEditor.Game.convertRotationRHtoLH( rot )

    #scaleMtx = LevelEditor.Game.Matrix33()
    #scaleMtx.createScaleMatrix( scale.x )

    rotationMtx = LevelEditor.Game.Matrix33()
    rotationMtx.createRotationMatrix( rot.x, rot.y, rot.z )

    mtx = rotationMtx

    if format == 'hierarchy':
        # Parent entity
        enti['Inherits'] = ['PropBase']

        # Model
        uri = gobj.getURI()
        if uri == None:
            print("GameObject (" + gobj.getName() + ") without URI found. Have you put a spawnpoint/item pop point, etc.. into Props?")
            assert(False)
        path, ext = os.path.splitext(uri)
        if ext == '.yml':
            # For instance
            if isInst:
                enti = createInstanceEntity( path, ext )
            else:
                # Change parent entity
                ymlName = os.path.basename( path )
                enti['Inherits'] = [ymlName]
        else:
            # Add model component
            outputUri = path + ".vmdc"
            outputUri = string.replace(outputUri, 'Models/', '')
            enti['ModelComponent'] = { 'name': outputUri }

        # Matrix
        enti['MatrixComponent'] = { 'matrix': { 'm': [mtx._11, mtx._12, mtx._13, 0.0,
                                                      mtx._21, mtx._22, mtx._23, 0.0,
                                                      mtx._31, mtx._32, mtx._33, 0.0,
                                                      trans.x, trans.y, trans.z, 1.0] } }

    elif format == 'positions':
        enti['name'] = gobj.getName()
        type = gobj.getType()
        teamNo = gobj.getAttribute('team')
        enti['userdata'] = 0

        locationType = 'UNKNOWN'
        if type == 'spawnPointType':
            locationType = 'TEAM_' + teamNo
        elif type == 'itemPopPointType':
            locationType = 'ITEM'
        elif type == 'aiWaypointType':
            locationType = 'AI_WAYPOINT'
        elif type == 'aiBlockingAreaType':
            locationType = 'AI_BLOCKING_AREA'
        elif type == 'flybyCamSplineControlPointType':
            locationType = 'CAMERA_CONTROL_POINT'
            ud = 0
            splineId = int(gobj.getAttribute('id'))
            order = int(gobj.getAttribute('order'))
            fVelocity = float(gobj.getAttribute('velocity'))
            if fVelocity < 0.1:
                fVelocity = 0.1
            if fVelocity > 5.0:
                fVelocity = 5.0
            iVelocity = int(255 * (fVelocity-0.1)/4.9)
            enti['userdata'] =  (splineId&0xff) | ((order&0xff) << 8) | ((iVelocity&0xff) << 16)

        enti['locationType'] = locationType
        rad2deg = 180 / math.pi
        enti['rotation'] = [ rot.x * rad2deg, rot.y * rad2deg, rot.z * rad2deg ]
        enti['scale'] = [ scale.x, scale.y, scale.z ]
        enti['translation'] = [ trans.x, trans.y, trans.z ]

    return enti

def gatherObjectsRecursively( nodes, folder, format ):
    # Instance?
    isInst = False
    if folder.attrib['name'] == 'Instances':
        isInst = True

    for obj in folder:
        # child folder?
        if obj.tag == "{gap}folder":
            gatherObjectsRecursively( nodes, obj, format )
            continue


        # Make an entity
        enti = makeEntity( obj, format, isInst )

        nodes.append( enti )

# format -> 'hierarchy' or 'positions'
def convert( folders, targetDirs, format ):

    nodes = []
    for folder in folders:
        folderName = folder.attrib['name']
        # if the folder name starts with any of the names in targetDirs
        if sum([(folderName == tD) or (folderName.startswith(tD) and folderName != tD) for tD in targetDirs]) > 0:
            # Make a tree
            gatherObjectsRecursively( nodes, folder, format )

    yamlTree = {}

    if format == 'hierarchy':
        yamlTree = nodes
    elif format == 'positions':
        yamlTree['Billboard'] = False
        yamlTree['Handedness'] = 'left'
        yamlTree['Instance'] = False
        yamlTree['Format'] = 'transform'
        yamlTree['Length'] = len( nodes )
        yamlTree['Nodes'] = nodes

    return yamlTree

def convertLevel ( lvl, targetDirs, format ):
    # Get all top-level folders, except for the sublevels folder if
    # present.
    folders = lvl.getFolders(['SubLevels'])

    return convert( folders, targetDirs, format )