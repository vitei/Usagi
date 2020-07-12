#ifndef CMDL_H
#define CMDL_H

#include "OwnSTLDecl.h"
#include "Engine/Graphics/Lights/LightSpec.pb.h"
#include <float.h>
#include <stdint.h>

namespace exchange {
	class Stream;
	class Shape;
	class Material;
	class Mesh;
	class Skeleton;
	class Animation;
}

class Cmdl
{
public:
	Cmdl( void );
	virtual ~Cmdl();

	struct Light
	{
		aya::string parentBone;
		aya::string name;
		usg::Vector3f position;
		usg::LightSpec spec;
	};

	struct Camera
	{
		aya::string		parentBone;
		aya::string		name;
		usg::Vector3f	position;
		usg::Vector3f	rotate;
		real			fov;
		real			nearPlane;
		real			farPlane;
	};

	void AddShape( ::exchange::Shape* p );
	void AddMaterial( ::exchange::Material* p );
	void AddMesh( ::exchange::Mesh* p );
	void AddAnimation(::exchange::Animation* p);
	void AddStream( ::exchange::Stream* p );
	void AddLight(Light* p);
	void AddCamera(Camera* p);
	void SetSkeleton( ::exchange::Skeleton* p );

	uint32_t GetShapeNum( void ) const { return (uint32_t)m_vectorShape.size(); }
	uint32_t GetMaterialNum( void ) const { return (uint32_t)m_vectorMaterial.size(); }
	uint32_t GetMeshNum( void ) const { return (uint32_t)m_vectorMesh.size(); }
	uint32_t GetStreamNum( void ) const { return (uint32_t)m_vectorStream.size(); }
	uint32_t GetAnimationNum(void) const { return (uint32_t)m_vectorAnimation.size(); }
	uint32_t GetLightNum(void) const { return (uint32_t)m_lightStream.size(); }
	uint32_t GetCameraNum(void) const { return (uint32_t)m_cameraStream.size(); }

	uint32_t GetBoneIndexCount(int materialNum);
	std::vector< uint8 >& GetRigidIndices() { return m_rigidBones; }
	std::vector< uint8 >& GetSmoothIndices() { return m_smoothSkinBones; }
	const std::vector< uint8 >& GetRigidIndices() const { return m_rigidBones; }
	const std::vector< uint8 >& GetSmoothIndices() const { return m_smoothSkinBones; }

	::exchange::Shape*		GetShapePtr( int i ) const { return m_vectorShape.at(i); }
	::exchange::Material*	GetMaterialPtr( int i ) const { return m_vectorMaterial.at(i); }
	::exchange::Mesh*		GetMeshPtr( int i ) const { return m_vectorMesh.at(i); }
	::exchange::Animation*	GetAnimation(int i) const { return m_vectorAnimation.at(i); }
	::exchange::Stream*		GetStreamPtr( int i ) const { return m_vectorStream.at( i ); }
	::exchange::Skeleton*	GetSkeleton( void ) const { return m_pSkeleton; }
	Light*					GetLight(int i) { return m_lightStream[i]; }
	Camera*					GetCamera(int i) { return m_cameraStream[i]; }

	void ReverseCoordinate( void );
	void CalculatePolygonNormal( void );

	aya::string GetName() const;
	void SetName(const aya::string& value);

private:
	template<class T> void DeleteAll( std::vector< T, aya::Allocator<T> >& v );
	void ReverseCoordinateInt(Light* pLight );
	void ReverseCoordinateInt(Camera* pCamera);
	void ReverseCoordinateInt(::exchange::Shape* pShape);
	void ReverseCoordinateInt( ::exchange::Stream& stream );
	void CalculatePolygonNormal( ::exchange::Shape* pShape );



	aya::string m_stringName;

	std::vector< ::exchange::Shape*, aya::Allocator< ::exchange::Shape*> >			m_vectorShape;
	std::vector< ::exchange::Material*, aya::Allocator< ::exchange::Material*> >	m_vectorMaterial;
	std::vector< ::exchange::Mesh*, aya::Allocator< ::exchange::Mesh* > >			m_vectorMesh;
	std::vector< ::exchange::Animation*, aya::Allocator< ::exchange::Animation* > >	m_vectorAnimation;
	std::vector< ::exchange::Stream*, aya::Allocator< ::exchange::Stream* > >		m_vectorStream;
	std::vector< Light*, aya::Allocator< Light* > >									m_lightStream;
	std::vector< Camera*, aya::Allocator< Camera* > >								m_cameraStream;

	std::vector< uint8 >															m_rigidBones;
	std::vector< uint8 >															m_smoothSkinBones;

	::exchange::Skeleton* m_pSkeleton;

};

#endif // CMDL_H
