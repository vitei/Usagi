/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/


template <class IndexType>
class GameHandle
{
public:
	GameHandle() { m_uValue = (IndexType)(-1); }
	GameHandle(IndexType uValue) { m_uValue = uValue; }
	~GameHandle() {}

	IndexType GetValue() const { return m_uValue; }
	bool operator==(const GameHandle& rhs) const { return m_uValue == rhs.m_uValue; }
	bool operator!=(const GameHandle& rhs) const { return m_uValue != rhs.m_uValue; }
	void Invalidate() { m_uValue = InvalidValue();  }

	static IndexType InvalidValue() { return IndexType(-1); }
protected:
	IndexType m_uValue;
};

// TODO: Extend this to a smart pointer system, for now just getting the interface in place
template <class PointerType, class IndexType>
class PointerHandle : public GameHandle<IndexType>
{
	typedef GameHandle<IndexType> Inherited;
public:
	PointerHandle() { m_pData = NULL; }
	PointerHandle(PointerType* pData, IndexType uValue) : GameHandle<IndexType>(uValue) { m_pData = pData; }
	~PointerHandle() {}

	PointerType* GetContents() const { return m_pData; }
	bool IsValid() const { return m_pData != NULL;  }
	void Invalidate() { Inherited::Invalidate(); m_pData = NULL; }

	bool operator==(const PointerHandle& rhs) const { return m_pData == rhs.m_pData; }
	bool operator!=(const PointerHandle& rhs) const { return m_pData != rhs.m_pData; }

private:
	PointerType* m_pData;
};

typedef GameHandle<uint8> GameHandle8;
typedef GameHandle<uint16> GameHandle16;
typedef GameHandle<uint32> GameHandle32;
