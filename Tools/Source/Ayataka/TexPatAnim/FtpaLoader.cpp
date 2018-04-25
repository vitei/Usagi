
#include "FtpaLoader.h"

#include "common.h"
#include "PbUtil.h"
#include "StringUtil.h"

namespace {
	struct Context {
		_TexPatMatAnim* pCurrentTexPatMatAnim;
	} mContext;

	void clearContext( Context& context ) {
		context.pCurrentTexPatMatAnim = NULL;
	}
}

void _callback_AnimInfo( TexPatAnim& texPatAnim, const pugi::xml_node& node )
{
	_TexPatAnimInfo& pb = texPatAnim.pb();
	SET_XML_ATTR_INTO_PB( length, "frame_count", int );
	SET_XML_ATTR_INTO_PB( startFrame, "dcc_start_frame", int );
	SET_XML_ATTR_INTO_PB( endFrame, "dcc_end_frame", int );
	SET_XML_ATTR_INTO_PB( isLoop, "loop", bool );
}

void _callback_TexPatternArray( TexPatAnim& texPatAnim, const pugi::xml_node& node )
{
	uint32_t length = node.attribute( "length" ).as_uint();
	texPatAnim.reserveTexPatArraySize( length );
}

void _callback_TexPattern( TexPatAnim& texPatAnim, const pugi::xml_node& node )
{
	uint32_t index = node.attribute( "pattern_index" ).as_uint();

	_TexPat& pb = texPatAnim.getTexPat( index );
	STRING_COPY( pb.textureName, getAttributeString( node, "tex_name" ) );
}

void _callback_TexPatternMatAnimArray( TexPatAnim& texPatAnim, const pugi::xml_node& node )
{
	uint32_t length = node.attribute( "length" ).as_uint();
	texPatAnim.reserveTexPatAnimArraySize( length );
}

void _callback_TexPatternMatAnim( TexPatAnim& texPatAnim, const pugi::xml_node& node )
{
	// todo: copy target material name

	uint32_t index = node.attribute( "index" ).as_uint();
	_TexPatMatAnim& pb = texPatAnim.getTexPatMatAnim( index );
	STRING_COPY( pb.materialName, getAttributeString( node, "mat_name" ) );

	mContext.pCurrentTexPatMatAnim = &pb;
}

void _callback_PatternAnimTarget( TexPatAnim& texPatAnim, const pugi::xml_node& node )
{
	_TexPatMatAnim* pCurrent = mContext.pCurrentTexPatMatAnim;

	uint32_t index = pCurrent->patAnimTargets_count;
	STRING_COPY( pCurrent->patAnimTargets[index].samplerName, node.attribute( "sampler_name" ).as_string() );

	pCurrent->patAnimTargets_count = ++index;
}

void _callback_StepCurve( TexPatAnim& texPatAnim, const pugi::xml_node& node )
{

}


int FtpaLoader::load( TexPatAnim& texPatAnim, const char* path )
{
	pugi::xml_document doc;
	if( !doc.load_file( path ) ) {
		// open failed
		return -1;
	}

	_loadTexPatAnim( texPatAnim, doc, "nw4f_3dif/tex_pattern_anim" );

	return 0;
}

void FtpaLoader::_loadTexPatAnim( TexPatAnim& texPatAnim, const pugi::xml_document& doc, const char* xpath )
{
	pugi::xpath_node node = evaluateXpathQuery( doc, xpath ).first();

	mTagCallbacks["tex_pattern_anim_info"] = &_callback_AnimInfo;
	mTagCallbacks["tex_pattern_array"] = &_callback_TexPatternArray;
	mTagCallbacks["tex_pattern"] = &_callback_TexPattern;

	mTagCallbacks["tex_pattern_mat_anim_array"] = &_callback_TexPatternMatAnimArray;
	mTagCallbacks["tex_pattern_mat_anim"] = &_callback_TexPatternMatAnim;

	mTagCallbacks["pattern_anim_target"] = &_callback_PatternAnimTarget;
	mTagCallbacks["step_curve"] = &_callback_StepCurve;

	_traverseRecursively( texPatAnim, node.node() );
}

