import math
import xml.etree.ElementTree as etree

myNamespaces = { 'gap' : 'gap' }
XmlSchemaInstance = "http://www.w3.org/2001/XMLSchema-instance"

class Matrix33:
    def __init__(self):
        self.setIdentity()

    def setIdentity(self):
        self._11 = 1.0
        self._12 = 0.0
        self._13 = 0.0
        self._21 = 0.0
        self._22 = 1.0
        self._23 = 0.0
        self._31 = 0.0
        self._32 = 0.0
        self._33 = 1.0

    def copy(self, mtx):
        self._11 = mtx._11
        self._12 = mtx._12
        self._13 = mtx._13
        self._21 = mtx._21
        self._22 = mtx._22
        self._23 = mtx._23
        self._31 = mtx._31
        self._32 = mtx._32
        self._33 = mtx._33

    def createRotationMatrix(self, x, y, z):
        self.setIdentity()

        cx = math.cos(x)
        sx = math.sin(x)
        cy = math.cos(y)
        sy = math.sin(y)
        cz = math.cos(z)
        sz = math.sin(z)

        self._11 = cy * cz
        self._12 = cy * sz
        self._13 = -sy

        self._21 = (sx * sy * cz) - (cx * sz)
        self._22 = (sx * sy * sz) + (cx * cz)
        self._23 = sx * cy

        self._31 = (cx * sy * cz) + (sx * sz)
        self._32 = (cx * sy * sz) - (sx * cz)
        self._33 = cx * cy

    def createRotationX(self, angle):
        self.setIdentity()

        cos = math.cos(angle)
        sin = math.sin(angle)

        self._22 = cos
        self._23 = sin
        self._32 = -sin
        self._33 = cos

    def createRotationY(self, angle):
        self.setIdentity()

        cos = math.cos(angle)
        sin = math.sin(angle)

        self._11 = cos
        self._13 = -sin
        self._31 = sin
        self._33 = cos

    def createRotationZ(self, angle):
        self.setIdentity()

        cos = math.cos(angle)
        sin = math.sin(angle)

        self._11 = cos
        self._12 = sin
        self._21 = -sin
        self._22 = cos

    def createScaleMatrix(self, scale):
        self.setIdentity()

        self._11 = scale.x
        self._12 = 0.0
        self._13 = 0.0

        self._21 = 0.0
        self._22 = scale.y
        self._23 = 0.0

        self._31 = 0.0
        self._32 = 0.0
        self._33 = scale.z

    def multiply(self, mtx):
        temp = Matrix33()
        temp.copy(self)
        self._11 = ( temp._11 * mtx._11 ) + ( temp._12 * mtx._21 ) + ( temp._13 * mtx._31 )
        self._12 = ( temp._11 * mtx._12 ) + ( temp._12 * mtx._22 ) + ( temp._13 * mtx._32 )
        self._13 = ( temp._11 * mtx._13 ) + ( temp._12 * mtx._23 ) + ( temp._13 * mtx._33 )

        self._21 = ( temp._21 * mtx._11 ) + ( temp._22 * mtx._21 ) + ( temp._23 * mtx._31 )
        self._22 = ( temp._21 * mtx._12 ) + ( temp._22 * mtx._22 ) + ( temp._23 * mtx._32 )
        self._23 = ( temp._21 * mtx._13 ) + ( temp._22 * mtx._23 ) + ( temp._23 * mtx._33 )

        self._31 = ( temp._31 * mtx._11 ) + ( temp._32 * mtx._21 ) + ( temp._33 * mtx._31 )
        self._32 = ( temp._31 * mtx._12 ) + ( temp._32 * mtx._22 ) + ( temp._33 * mtx._32 )
        self._33 = ( temp._31 * mtx._13 ) + ( temp._32 * mtx._23 ) + ( temp._33 * mtx._33 )


class Vector3:
    def __init__(self):
        pass

    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

def splitVectorAttrib( node, attribName ):
    str = node.attrib[attribName]
    split = str.split(' ')
    return Vector3( float( split[0] ), float( split[1] ), float( split[2] ) )

def convertTranslationRHtoLH( trans ):
    trans.x = trans.x * -1.0
    return trans

def convertRotationRHtoLH( rot ):
    rot.y = rot.y * -1.0
    rot.z = rot.z * -1.0
    return rot

def calcDistance( pointA, pointB ):
    sq = math.pow( pointA.x - pointB.x, 2 ) + math.pow( pointA.y - pointB.y, 2 ) + math.pow( pointA.z - pointB.z, 2 )
    return math.sqrt( sq )

def fillQuaternionFromMatrix( mtx ):
	quaternion = { 'x' : 0.0, 'y' : 0.0, 'z' : 0.0, 'w' : 0.0 }
	trace = mtx._11 + mtx._22 + mtx._33 + 1.0

	if (trace > 0.0):
		quaternion['x'] = (mtx._23 - mtx._32) / (2.0 * math.sqrt(trace))
		quaternion['y'] = (mtx._31 - mtx._13) / (2.0 * math.sqrt(trace))
		quaternion['z'] = (mtx._12 - mtx._21) / (2.0 * math.sqrt(trace))
		quaternion['w'] = math.sqrt(trace) / 2.0

		return quaternion

	maxI = '_11'
	maxDiag = mtx._11

	for element, value in zip(['_22', '_33'], [mtx._22, mtx._33]):
		if value > maxDiag:
			maxI = element
			maxDiag = value

	if maxI == '_11':
		S = 2.0 * math.sqrt(1.0 + mtx._11 - mtx._22 - mtx._33)
		quaternion['x'] = 0.25 * S
		quaternion['y'] = (mtx._12 + mtx._21) / S
		quaternion['z'] = (mtx._13 + mtx._31) / S
		quaternion['w'] = (mtx._23 - mtx._32) / S
	elif maxI == '_22':
		S = 2.0 * math.sqrt(1.0 + mtx._22 - mtx._11 - mtx._33)
		quaternion['x'] = (mtx._12 + mtx._21) / S
		quaternion['y'] = 0.25 * S
		quaternion['z'] = (mtx._23 + mtx._32) / S
		quaternion['w'] = (mtx._31 - mtx._13) / S
	elif maxI == '_33':
		S = 2.0 * math.sqrt(1.0 + mtx._33 - mtx._11 - mtx._22)
		quaternion['x'] = (mtx._13 + mtx._31) / S
		quaternion['y'] = (mtx._23 + mtx._32) / S
		quaternion['z'] = 0.25 * S
		quaternion['w'] = (mtx._12 - mtx._21) / S

	return quaternion

class GameObject:
    def __init__(self, gobjNode):
        self.node = gobjNode

    def getTranslation(self):
        return splitVectorAttrib( self.node, 'translate' )

    def setTranslation(self, vec):
        self.setAttribute( "translate", '%f %f %f' % ( vec.x, vec.y, vec.z ) )

    def getRotate(self):
        return splitVectorAttrib( self.node, 'rotate' )

    def setRotate(self, vec):
        self.setAttribute( "rotate", '%f %f %f' % ( vec.x, vec.y, vec.z ) )

    def getScale(self):
        return splitVectorAttrib( self.node, 'scale' )

    def setScale(self, vec):
        self.setAttribute( "scale", '%f %f %f' % ( vec.x, vec.y, vec.z ) )

    def setTransform(self, mtx, vec):
        mtxString = '%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f' % (
            mtx._11, mtx._12, mtx._13, 0.0,
            mtx._21, mtx._22, mtx._23, 0.0,
            mtx._31, mtx._32, mtx._33, 0.0,
            vec.x, vec.y, vec.z, 1.0)
        self.setAttribute( "transform", mtxString )

    def getChildResource(self):
        return self.node.find( 'gap:resource', myNamespaces )

    def getName(self):
        return self.node.attrib['name']

    def getType(self):
        return self.node.attrib['{' + XmlSchemaInstance + '}type']

    def setType(self, type):
        self.node.attrib['{' + XmlSchemaInstance + '}type'] = type

    def getAttribute(self, attrName):
        if self.node.attrib.has_key(attrName):
            return self.node.attrib[attrName]
        else:
            return None

    def setAttribute(self, attrName, value):
        self.node.attrib[attrName] = value

    def getVisibility(self):
        return self.node.attrif['visible']

    def getURI(self):
        resNode = self.getChildResource()
        if resNode is None:
            return None
        else:
            return resNode.attrib['uri']

    def setURI(self, uri):
        resNode = self.getChildResource()
        resNode.attrib['uri'] = uri


class Level:
    def __init__(self, xmlPath):
        etree.register_namespace( '', 'gap' )
        self.xmlDoc = etree.parse( open( xmlPath, 'r' ) )
        self.root = self.xmlDoc.getroot()
        self.gameObjectFolder = self.root.find( 'gap:gameObjectFolder', myNamespaces )

    def getFolders(self, ignoredFolders = []):
        ignoredFoldersInsensitive = [f.lower() for f in ignoredFolders]
        folders = self.gameObjectFolder.findall( './gap:folder', myNamespaces )
        folders = [f for f in folders if not f.attrib['name'].lower() in ignoredFoldersInsensitive]
        return folders

    def searchFolder(self, name):
        return self.gameObjectFolder.find( "gap:folder[@name='%s']" % name, myNamespaces )

    def searchGameObject(self, name):
        # Search from entire XML
        return self.gameObjectFolder.find( ".//gap:gameObject[@name='%s']" % name, myNamespaces )

    def dump(self):
        print self.root.tag
        folders = self.getFolders()
        depth = 0
        for folder in folders:
            self.dumpFolder( folder, depth )

    def dumpFolder(self, folder, depth):
        print '----'
        print folder.attrib['name'] + ' depth:%d' % depth
        print '----'
        for obj in folder:
            if obj.tag == "{gap}folder":
                self.dumpFolder( obj, depth + 1 )
                continue

            print obj.tag

            gobj = GameObject(obj)
            trans = gobj.getTranslation()
            rot = gobj.getRotate()
            scale = gobj.getScale()

            print "name:" + gobj.getName()
            print "trans %f, %f, %f" % ( trans.x, trans.y, trans.z )
            print "rot   %f, %f, %f" % ( rot.x, rot.y, rot.z )
            print "scale %f, %f, %f" % ( scale.x, scale.y, scale.z )
            uri = gobj.getURI()
            if not uri == None:
                print 'uri   ' + uri
