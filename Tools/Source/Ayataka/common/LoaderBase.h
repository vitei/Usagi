#ifndef LoaderBase_h__
#define LoaderBase_h__


#include <map>

template <class T>
class LoaderBase {
protected:
	typedef void( *TagCallback )( T&, const pugi::xml_node& );
	typedef std::map<aya::string, TagCallback> MapTagCallback;
	MapTagCallback mTagCallbacks;

	void TraverseRecursively( T& cache, const pugi::xml_node& node ) const
	{
		if( node.empty() ) { return; }

		typename MapTagCallback::const_iterator itr = mTagCallbacks.find( node.name() );
		if( itr != mTagCallbacks.end() ) {
			DEBUG_PRINT( "<%s>\n", node.name() );
			TagCallback pCallbackFunc = itr->second;
			( *pCallbackFunc )( cache, node );
		}

		pugi::xml_node child = node.first_child();
		while( !child.empty() ) {
			TraverseRecursively( cache, child );
			child = child.next_sibling();
		}
	}
};

#endif // LoaderBase_h__
