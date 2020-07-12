/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An element in a parent child hierarchy
*****************************************************************************/
#ifndef _USG_FRAMEWORK_HIERARCHY_NODE_
#define	_USG_FRAMEWORK_HIERARCHY_NODE_


namespace usg 
{

template <class T>
class HierearchyNode
{
public:
	HierearchyNode();
	~HierearchyNode() { /*DetachSelf();*/ }

	T* GetParentEntity() { return m_pParent; }
	T* GetChildEntity() { return m_pChild; }
	T* GetNextSibling() { return m_pNextSibling; }

	void ClearHierarchy();
	void AttachToNode(T* pParent);
	void DetachSelf(bool bDetachChildren = true);
	bool IsChildOf(T* pParent);

protected:
	PRIVATIZE_COPY(HierearchyNode)

	T*	m_pParent;
	T*	m_pChild;
	T*	m_pPrevSibling;
	T*	m_pNextSibling;
};


template <class T>
inline HierearchyNode<T>::HierearchyNode()
{
	ClearHierarchy();
}

template <class T>
inline void HierearchyNode<T>::ClearHierarchy()
{
	m_pParent		= NULL;
	m_pChild		= NULL;
	m_pPrevSibling	= NULL;
	m_pNextSibling	= NULL;
}

template <class T>
inline void HierearchyNode<T>::AttachToNode(T* pParent)
{
	m_pParent = pParent;
	if(pParent!=NULL)
	{
		if(!pParent->m_pChild)
		{		
			// Don't have a child yet so just slot us in
			pParent->m_pChild = (T*)this;
		}
		else
		{
			T* pPrevChild = pParent->m_pChild;
			pParent->m_pChild = (T*)this;
			m_pNextSibling = pPrevChild;
			pPrevChild->m_pPrevSibling = (T*)this;
		}
	}
}

template <class T>
bool HierearchyNode<T>::IsChildOf(T* pParent)
{
	T* pCmpParent = m_pParent;
	while (pCmpParent)
	{
		if (pCmpParent == pParent)
		{
			return true;
		}
		pCmpParent = pCmpParent->GetParentEntity();
	}

	return false; 
}

template <class T>
inline void HierearchyNode<T>::DetachSelf(bool bDetachChildren)
{
	T* pParent		= m_pParent;
	T* pChild		= m_pChild;
	T* pPrevSibling = m_pPrevSibling;
	T* pNextSibling = m_pNextSibling;

	if(pChild && bDetachChildren)
	{
		// Remove children, should only happen in cleanups
		pChild->DetachSelf();
		//ASSERT(false);
		DEBUG_PRINT("WARNING: Removing child before parent\n");
		m_pChild = NULL;
	}

	if(pParent && (pParent->m_pChild == this))
	{
		pParent->m_pChild = pNextSibling;
	}

	if(pNextSibling)
	{
		pNextSibling->m_pPrevSibling = pPrevSibling;
		pNextSibling->m_pParent = pParent;
	}

	if(pPrevSibling)
	{
		pPrevSibling->m_pNextSibling = pNextSibling;
		pPrevSibling->m_pParent = pParent;
	}

	m_pPrevSibling	= NULL;
	m_pNextSibling	= NULL;
	m_pParent		= NULL;
}


}

#endif
