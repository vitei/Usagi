/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PROTOCOL_BUFFER_VAR_LENGTH_H__
#define _USG_PROTOCOL_BUFFER_VAR_LENGTH_H__

#include <pb.h>
#include <pb_decode.h>


#include "Engine/Core/ProtocolBuffers/ProtocolBufferFields.h"

namespace usg {

// Helper to make pb_callback_t a bit easier to use with C++
// You need to define a Delegate class, which is a template class parameterised on
// the type being decoded.  Delegates need to define the following:
//   - A function void init(void** arg); initialising the free arg for use
//   - A function void finalise(void** arg); which performs any final cleanup
//     and points arg at the loaded structure.
//   - A typedef AccessorType which defines the type of the collection, for example
//       T*, List<T>, etc.
//   - A public member data of type AccessorType, which holds the handle into the collection
//     itself; i.e. for a C array it holds a pointer, for a linked list a reference to the
//     first entry, etc.
//   - A struct called WorkingData to store any data required during decode, or a
//     typedef of the type you're writing to WorkingData if everything is stored in
//     the class itself.  You are responsible for allocating WorkingData in your init()
//     function and clearing it up in finalise().
//   - The WorkingData type must provide a function called alloc() which allocates space
//     for one field and returns a pointer to it, and a function increment() which gets run
//     after each field is decoded (and can be empty).
// Since the ProtocolBufferVarLength stores one of these Delegates locally, it's
// best not to store too much data inside it -- ideally, only store the "data" handle.
// for load-time working data, you can initialise a local WorkingData struct and clear
// it up in finalise().  See PBChunkedArray for an example.
template < typename T, class Delegate >
struct ProtocolBufferVarLength : public pb_callback_t
{
	ProtocolBufferVarLength() { arg = NULL; funcs.decode = decode; }

	      typename Delegate::AccessorType& Get()       { return m_decoderDelegate.data; }
	const typename Delegate::AccessorType& Get() const { return m_decoderDelegate.data; }

	// If the Delegate's AccessorType provides a subscript operator, you can access it
	      T& operator[](int i)       { return Get()[i]; }
	const T& operator[](int i) const { return Get()[i]; }

	// If the Delegate's AccessorType provides an iterator, you can access it
	// by adding a line "typedef AccessorType::Iterator Iterator" to your Delegate.
	typedef typename Delegate::Iterator Iterator;
	Iterator Begin() const { return Get().Begin(); }
	Iterator End() const { return Get().End(); }

	// Called before and after decode().  This should be all wrapped up in the
	// template machinery, so you shouldn't need to call them manually.
	void PreLoad()  { m_decoderDelegate.init(&arg); }
	void PostLoad() { m_decoderDelegate.finalise(&arg); }

	// Called before and after encode().  Only delegates which provide an Iterator can
	// be encoded.
	void PreSave()  { arg = this; funcs.encode = encode; }
	void PostSave() { arg = NULL; funcs.decode = decode; }

	// The delegate used to do the decoding is publicly exposed in case you need to
	// set up an InitializeFieldCB callback.
	Delegate m_decoderDelegate;

private:
	static bool decode(pb_istream_t *stream, const pb_field_t *field, void **arg)
	{
		ASSERT(*arg != NULL);

		typename Delegate::WorkingData *decoder = (typename Delegate::WorkingData*)*arg;
		while(stream->bytes_left)
		{
			T* current = decoder->alloc();

			ProtocolBufferFields<T>::PreLoad(current);
			decoder->InitializeField(current);
			bool success = pb_decode(stream, ProtocolBufferFields<T>::Spec(), current);
			ProtocolBufferFields<T>::PostLoad(current);

			if(!success)
				return false;

			decoder->increment();
		}

		return true;
	}

	static bool encode(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
	{
		ASSERT(*arg != NULL);
		ProtocolBufferVarLength<T, Delegate>* self = (ProtocolBufferVarLength<T, Delegate>*)*arg;

		bool success;
		for(ProtocolBufferVarLength<T, Delegate>::Iterator i = self->Begin(); !i.IsEnd(); ++i)
		{
			T* current = &(*i);

			success = pb_encode_tag_for_field(stream, field);
			if(success)
			{
				//TODO repeated string fields might be problematic here...
				//     nanopb docs say:
				//       "if you want to encode a repeated field as a packed array, you must call pb_encode_tag instead to specify a wire type of PB_WT_STRING"
				//       http://koti.kapsi.fi/jpa/nanopb/docs/concepts.html#encoding-callbacks

				ProtocolBufferFields<T>::PreSave(current);
				success = pb_encode_submessage(stream, ProtocolBufferFields<T>::Spec(), current);
				ProtocolBufferFields<T>::PostSave(current);
			}

			if(!success)
				return false;
		}

		return true;
  }
};

}

#endif //_USG_PROTOCOL_BUFFER_VAR_LENGTH_H__

