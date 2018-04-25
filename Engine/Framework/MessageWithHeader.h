/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Gives us a slightly hacky way to encode messages with an
//	accompanying header... hacky because we generate the ID by XORing the
//	header type ID with the payload type ID, which may result in clashes :-/
****************************************************************************/

#ifndef MESSAGE_WITH_HEADER_H
#define MESSAGE_WITH_HEADER_H

namespace usg {

template<typename HeaderType, typename PayloadType>
struct MessageWithHeader
{
	HeaderType  hdr;
	PayloadType body;
};

template <typename HeaderType, typename PayloadType>
struct ProtocolBufferFields< MessageWithHeader<HeaderType, PayloadType> > {
	// Let's just hope to God this never conflicts with an actual PB id :-/
	enum { ID = ProtocolBufferFields<HeaderType>::ID ^ ProtocolBufferFields<PayloadType>::ID };

	static void PreLoad(MessageWithHeader<HeaderType, PayloadType> *msg) {
		ProtocolBufferFields<HeaderType>::PreLoad(&msg->hdr);
		ProtocolBufferFields<PayloadType>::PreLoad(&msg->body);
	}
	static void PostLoad(MessageWithHeader<HeaderType, PayloadType> *msg) {
		ProtocolBufferFields<HeaderType>::PostLoad(&msg->hdr);
		ProtocolBufferFields<PayloadType>::PostLoad(&msg->body);
	}
	static void PreSave(MessageWithHeader<HeaderType, PayloadType> *msg) {
		ProtocolBufferFields<HeaderType>::PreSave(&msg->hdr);
		ProtocolBufferFields<PayloadType>::PreSave(&msg->body);
	}
	static void PostSave(MessageWithHeader<HeaderType, PayloadType> *msg) {
		ProtocolBufferFields<HeaderType>::PostSave(&msg->hdr);
		ProtocolBufferFields<PayloadType>::PostSave(&msg->body);
	}
};

// It is not valid to pass the same type as both header and payload
template <typename T> struct ProtocolBufferFields< MessageWithHeader<T, T> > {};

}

#endif //MESSAGE_WITH_HEADER_H
