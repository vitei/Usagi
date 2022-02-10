/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Decoder delegate for reading into a U8String.
*****************************************************************************/
#ifndef _USG_CORE_PB_U8STRING_H_
#define _USG_CORE_PB_U8STRING_H_


#include <pb.h>

namespace usg
{

	class U8String;

	template <int BUFFER_SIZE = 128>
	struct PBU8String : public pb_callback_t
	{
		struct WorkingData;
		WorkingData* m_data;

		PBU8String();
		PBU8String(const PBU8String& o) = delete;
		PBU8String& operator=(const PBU8String& o) = delete;
		~PBU8String();

		usg::string& Get();
		const usg::string& Get() const;

		void PreLoad();
		void PostLoad();
		void PreSave();
		void PostSave();
	private:
		static bool decode(pb_istream_t *stream, const pb_field_t *field, void **arg);
		static bool encode(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
	};

}

#endif // _USG_CORE_PB_U8STRING_H_
