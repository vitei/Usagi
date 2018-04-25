import sys, os, math

import EntityYaml
reload( EntityYaml )

import LevelEditor
reload( LevelEditor )

import maya.cmds as cmds
reload( cmds )

import MayaUtil as maya
reload( maya )

groupIndices = {}

def getGroupIndexAndAccumulate( name ):
    groupIndex = 0
    if name in groupIndices:
        groupIndex = groupIndices[name]

    # Accumulate group index
    groupIndices[name] = groupIndex + 1
    return groupIndex

##
# Add new GameObject to Maya scene
def addGameObject( gameObjectElem, parentGroupName ):
    gobj = LevelEditor.Game.GameObject(gameObjectElem)

    # Split uri to Name and Extension
    uri = gobj.getURI()
    if uri is None:
        # This is not a GameObject which has resource as a child
        name = 'GameObject_wo_res'

        # Group index accumulator
        groupIndex = getGroupIndexAndAccumulate( name )
        newGroupName = name + '_grp' + str(groupIndex)

        # Create own group and move to under the group named parentGroupName
        maya.createChildGroup( newGroupName, parentGroupName )

        # Add a red cube as a marker
        maya.addColoredCube( 1, 0, 0 )
        groupNewCube = maya.getCurrentSelection()
        maya.setParent( groupNewCube, newGroupName )

        # Add Name attribute
        maya.addAndSetAttribute( newGroupName, 'name', gobj.getName() )
    else:
        path, basename = os.path.split(uri)
        name, ext = os.path.splitext( basename )
        
        if ext == '.yml':
            path = EntityYaml.searchModelComponent( uri )
            path, name = os.path.split( path )
            ext = '.cmdl'

        # Group index accumulator
        groupIndex = getGroupIndexAndAccumulate( name )
        newGroupName = name + '_grp' + str(groupIndex)

        # Create own group and move to under the group named parentGroupName
        maya.createChildGroup( newGroupName, parentGroupName )

        # Add Name attribute
        maya.addAndSetAttribute( newGroupName, 'name', gobj.getName() )

        # import a Maya scene file
        projectWorkspaceDir = './'

        # use the environment variable
        tank_data_dir = os.getenv( 'TANK_DATA_DIR' )
        projectWorkspaceDir = tank_data_dir

        # use the workspace directory?
        # projectWorkspaceDir = cmds.workspace( query=True, rootDirectory=True )
        
        MbPath = projectWorkspaceDir + path + '/scenes/' + name + '.mb'
        MbNamespace = name + '_ns' + str(groupIndex)
        MbGroupName = 'lvl_actualModel'
        ret = maya.importMayaBinary( MbPath, MbNamespace, MbGroupName )

        if ret:
            # This model's group is located on root firstly, so must move to under own one.
            maya.setParent( '|'+MbGroupName, newGroupName )

    # Move to the position
    trans = gobj.getTranslation()
    rot   = gobj.getRotate()
    maya.setTranslation( newGroupName, trans.x, trans.y, trans.z )
    toDeg = ( 180 / math.pi )
    maya.setRotation( newGroupName, rot.x * toDeg, rot.y * toDeg, rot.z * toDeg )

##
# Load xml attributes into Maya scene group
def loadAttributes( xmlElem, groupName ):
    invalidAttrList = [ 'transform', 'translate', 'rotate', 'scale' ]

    maya.setCurrentGroup( groupName )
    maya.addAndSetAttribute( groupName, 'xml_tag', xmlElem.tag )

    for key, value in xmlElem.items():
        xsiNamespace = '{' + LevelEditor.Game.XmlSchemaInstance + '}'
        if key.startswith( xsiNamespace ):
            key = key.replace( xsiNamespace, 'xsi_' )

        if not key in invalidAttrList:
            maya.addAndSetAttribute( groupName, key, value )



##
# Create folder tree recursively
def createFolderTree( folder, parentGroupName ):
    groupName = folder.attrib['name']
    maya.createNewGroup( groupName )
    maya.setParent( groupName, parentGroupName )

    for obj in folder:
        # Scan sub-folders
        if obj.tag == '{gap}folder':
            createFolderTree( obj, groupName )
            continue
        
        # Add GameObject
        addGameObject( obj, groupName )

##
# Create category groups
def createCategoryGroup( lvl, parentGroupName ):
    # Make a group for each category
    folders = lvl.getFolders()
    for folder in folders:
        createFolderTree( folder, parentGroupName )

def importLevel( lvl ):
    groupIndices.clear()
    createCategoryGroup( lvl, 'lvl' )