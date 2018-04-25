import maya.cmds as cmds

#
# Maya utility functions
#

def getCurrentSelection():
    return cmds.ls(sl=True)

def createChildGroup( newChildGroupName, parentGroupName ):
    cmds.group( empty=True, name=newChildGroupName )
    cmds.parent( newChildGroupName, parentGroupName )
    cmds.select( newChildGroupName, r=True )

def addAndSetAttribute( groupName, attrName, attr ):
    cmds.addAttr( groupName, ln=attrName, dataType='string' )
    cmds.setAttr( groupName+'.'+attrName, attr, type='string' )

def addNameAttribute( groupName, name ):
    cmds.addAttr( groupName, ln='name', dataType='string' )
    cmds.setAttr( groupName+'.name', name, type='string' )

def createNewGroup( groupName ):
    cmds.group( empty=True, name=groupName )

def setParent( child, newParent ):
    cmds.parent( child, newParent )

def setCurrentGroup( groupName ):
    cmds.select( groupName, r=True )

def setTranslation( groupName, x, y, z ):
    cmds.move( x, y, z, groupName )

def setRotation( groupName, x, y, z ):
    cmds.rotate( x, y, z, groupName )

def addColoredCube( r, g, b ):
    cmds.polyCube()
    cmds.setAttr( "lambert1.color", r, g, b, type='double3' )

def importMayaBinary( path, namespace, groupName ):
    try:
        cmds.file(path, i=True, type='mayaBinary',
                  ignoreVersion=True, ra=True, mergeNamespacesOnClash=False, namespace=namespace,
                  pr=True, loadReferenceDepth='none', gr=True, groupName=groupName )
        return True
    except:
        #raise RuntimeError('mb file :' + path + ' is not found!' )
        return False