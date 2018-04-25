import sys, os, math, re
import maya.cmds as cmds
import xml.etree.ElementTree as etree
import xml.dom.minidom as minidom

import LevelEditor
reload( LevelEditor )

##
def storeTransformMatrix( xmlElem, translate, rotate, scale ):
    rotationMtx = LevelEditor.Game.Matrix33()
    rotationMtx.createRotationMatrix( rotate.x, rotate.y, rotate.z )

    mtx = rotationMtx
    xmlElem.attrib['transform'] = '%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f' % (
        mtx._11, mtx._12, mtx._13, 0.0,
        mtx._21, mtx._22, mtx._23, 0.0,
        mtx._31, mtx._32, mtx._33, 0.0,
        translate.x, translate.y, translate.z, 1.0)

##
def storeVectorAttribute( xmlElem, groupName, attrName, value ):
    vector = cmds.getAttr( groupName + '.' + attrName )
    temp = ( vector[0][0] * value, vector[0][1] * value, vector[0][2] * value )
    xmlElem.attrib[attrName] = '%f %f %f' % ( temp[0], temp[1], temp[2] )
    return LevelEditor.Game.Vector3( temp[0], temp[1], temp[2] )

##
# Store attributes of scale, rotate, translate to Element
def storeSRTAttributes( xmlElem, groupName ):
    translate = storeVectorAttribute( xmlElem, groupName, 'translate', 1.0 )
    toRad = ( math.pi / 180 )
    rotate = storeVectorAttribute( xmlElem, groupName, 'rotate', toRad )
    scale = storeVectorAttribute( xmlElem, groupName, 'scale', 1.0 )

    storeTransformMatrix( xmlElem, translate, rotate, scale )

##
# Store user attributes to Element
def storeAttributes( xmlElem, groupName ):
    if xmlElem.tag.endswith( 'gameObject' ):
        storeSRTAttributes( xmlElem, groupName )

    ignoreList = ( 'xml_tag' )

    attrs = cmds.listAttr( userDefined=True )
    for attr in attrs:
        if attr in ignoreList:
            continue

        value = cmds.getAttr( groupName+'.'+attr )

        attr = attr.replace( 'xsi_', '{' + LevelEditor.Game.XmlSchemaInstance + '}' )
        xmlElem.attrib[attr] = value

##
# Store visibility attribute to Element
def storeVisibility( elem, groupName ):
    elem.attrib['visible'] = 'true'
    if cmds.getAttr( groupName+'.visibility' ) == 0:
        elem.attrib['visible'] = 'false'


# Create GameObject XML element
def createGameObjectElement( mayaGroupName ):
    cmds.select( mayaGroupName, r=True )
    xml_tag = cmds.getAttr( mayaGroupName+'.xml_tag' )

    childElem = etree.Element( xml_tag )
    storeAttributes( childElem, mayaGroupName )

    return childElem

def scanGroupsRecursively( lvl, parentName, path ):
    cmds.select( path + parentName, r=True )

    children = cmds.listRelatives( children=True )
    if children is None:
        return

    for child in children:
        # ignore a model
        if child == 'lvl_actualModel':
            continue

        groupName = path + parentName + '|' + child
        try:
            # GameObject group
            # Search XML element by its name and store transform info
            uniqueName = cmds.getAttr( groupName+'.name' )
            gameObjectElem = lvl.searchGameObject( uniqueName )
            storeSRTAttributes( gameObjectElem, groupName )
        except:
            pass

        scanGroupsRecursively( lvl, child, path + parentName + '|' )

def exportLevel( path ):
    # Can export only path already exists
    if not os.path.exists( path ):
        return

    # Make sure 'lvl' group exists
    rootGroupName = 'lvl'
    if not cmds.objExists( rootGroupName ):
        return

    cmds.select( rootGroupName, r=True )

    etree.register_namespace( '', 'gap' )
    lvl = LevelEditor.Game.Level(path) # Parse a template
    scanGroupsRecursively( lvl, rootGroupName, '|' )

    # Print prettified XML
    rough_string = etree.tostring( lvl.root, 'utf-8' )
    reparsed = minidom.parseString( rough_string )
    prettified = reparsed.toprettyxml('\t', '\n', 'utf-8')

    # 'toprettyxml' doesn't work against pre-prettified XML. Then delete annoying blank rows manually.
    prettified = re.sub( r'(?m)^[\t]+\n', '', prettified )

    f1 = open(path, 'w')
    f1.write( prettified )
    f1.close()
