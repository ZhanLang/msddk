#pragma once

#include"ktypes.h"
namespace msddk{;


template<class KEY, class TYPE> 
class CKMapNode
{
public:
	TYPE    Value;
	KEY	    Key;

public:
	typedef CKMapNode<KEY,TYPE> Node;
	enum  NodeColor {RED,BLACK};
	Node *  Parent;
	Node *  RightChild;
	Node *  LelfChild;
	BOOLEAN IsSnil;
	unsigned char    Color;
	
};

template<class KEY, class TYPE> 
class CKMapIterator								//Ϊ��֧�ֶ�map���а�����������
{
public:
	typedef CKMapNode<KEY,TYPE> Node;
	Node * pRBT_Node;
	CKMapIterator(){pRBT_Node=NULL;};
	CKMapIterator(Node* pNode){pRBT_Node=pNode;};
	void Increment();
	void Decrement();
	void operator++(int){ Increment();}				//������KEY
	void operator--(int){ Decrement();}				//���С��KEY
	operator Node* () const { return pRBT_Node; }
	Node* operator -> () const {return pRBT_Node;}
};



template<typename KEY>
struct CKMapNodLess
{
	bool operator ()(const KEY& l , const KEY& r) const
	{
		return l < r;
	}
};

template<class KEY, class TYPE, class _CMP = CKMapNodLess<KEY>> 
class CKMap
{
public:
	typedef CKMapIterator<KEY,TYPE> Iterator;
	typedef CKMapNode<KEY,TYPE>*     ValueType;
	typedef CKMapNode<KEY,TYPE> Node;
	inline unsigned int Count();
	
	CKMap();
	~CKMap();
	CKMap(const CKMap &v);
	VOID  Clear();
	BOOL  Insert(const KEY& Key, const TYPE& Value);
	BOOL  Remove(const KEY& Key);
	inline Node* Find(const KEY& Key);
	Node* Min() const;						  //������СKey�ڵ�,�Ա�ʹ��TRB_TreeIter ���б���
	Node* Max() const;						  //�������Key�ڵ�,�Ա�ʹ��TRB_TreeIter ���б���

	CKMap& operator= (const CKMap& t);
private:
	VOID  DeleteFixUp(Node *TreeNode);	  //ɾ�������ƽ��
	VOID  DeleteTree(Node * TreeNode);	  //ɾ��ָ������
	VOID  LeftRotate(Node * TreeNode);    //��ָ�����������,��������ָ��������Һ��ӽ��.
	VOID  RightRotate(Node * TreeNode);   //��ָ�����������,��������ָ����������ӽ��.

	Node *  m_Head;
	Node    m_Snil;
	UINT    m_Count;
};
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
/*ʹ�÷�ʽ
CKMap<CApiString, CApiString> strMap;
strMap.Insert(_T("5"), _T("hello"));
strMap.Insert(_T("4"), _T("mpimap"));

//����
CKMap<CApiString,CApiString>::Iterator it = strMap.Min();
for (; it ; it++)
{
}

//����
CKMap<CApiString,CApiString>::ValueType value = strMap.Find(_T("2"));
*/
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
template<class KEY, class TYPE> 
void CKMapIterator<KEY,TYPE>::Increment()			
{
	if(pRBT_Node==NULL)
		return;

	if(pRBT_Node->IsSnil)
	{	//����ڵ��ǿ�
		pRBT_Node=NULL;
		return;
	}
	else
	{	
		if(!pRBT_Node->RightChild->IsSnil)
		{	//����ڵ����ҽڵ�,�ҳ����ҽڵ����Сֵ
			pRBT_Node=pRBT_Node->RightChild;
			while(!pRBT_Node->LelfChild->IsSnil)
				pRBT_Node=pRBT_Node->LelfChild;
		}
		else
		{   //����ڵ��ҽڵ�Ϊ��
			while(pRBT_Node->Parent&&pRBT_Node->Parent->RightChild==pRBT_Node)
				pRBT_Node=pRBT_Node->Parent;
			pRBT_Node=pRBT_Node->Parent;
		}
	}
}

template<class KEY, class TYPE> 
void CKMapIterator<KEY,TYPE>::Decrement()
{
	if(pRBT_Node==NULL)
		return;

	if(pRBT_Node->IsSnil)
	{	//����ڵ��ǿ�
		pRBT_Node=NULL;
		return;
	}
	else
	{	
		if(!pRBT_Node->LelfChild->IsSnil)
		{	//����ڵ����ҽڵ�,�ҳ����ҽڵ����Сֵ
			pRBT_Node=pRBT_Node->LelfChild;
			while(!pRBT_Node->RightChild->IsSnil)
				pRBT_Node=pRBT_Node->RightChild;
		}
		else
		{   //����ڵ��ҽڵ�Ϊ��
			while(pRBT_Node->Parent&&pRBT_Node->Parent->LelfChild==pRBT_Node)
				pRBT_Node=pRBT_Node->Parent;
			pRBT_Node=pRBT_Node->Parent;
		}
	}
}

/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
template<class KEY, class TYPE, class _CMP> 
CKMapNode<KEY,TYPE>*  CKMap<KEY,TYPE,_CMP>::Min() const
{
	Node* pResult=m_Head;
	if(!pResult->IsSnil)
	{
		while(!pResult->LelfChild->IsSnil)
			pResult=pResult->LelfChild;
	}

	if(pResult->IsSnil)
		return NULL;
	return pResult;
}

template<class KEY, class TYPE, class _CMP> 
CKMapNode<KEY,TYPE>*  CKMap<KEY,TYPE,_CMP>::Max() const 
{
	Node* pResult=m_Head;
	if(!pResult->IsSnil)
	{
		while(!pResult->RightChild->IsSnil)
			pResult=pResult->RightChild;
	}

	if(pResult->IsSnil)
		return NULL;
	return pResult;
}

template<class KEY, class TYPE, class _CMP> 
CKMapNode<KEY,TYPE>*  CKMap<KEY,TYPE,_CMP>::Find(const KEY& Key)
{
	Node* pResult=m_Head;
	while(!pResult->IsSnil)
	{
		if(pResult->Key==Key)			//��Key����
			return pResult;

		if(_CMP()(Key,pResult->Key))
			pResult=pResult->LelfChild;
		else
			pResult=pResult->RightChild;
	}

	if(pResult->IsSnil)
		pResult=NULL;

	return pResult;
}

template<class KEY, class TYPE, class _CMP> 
BOOL  CKMap<KEY,TYPE,_CMP>::Insert(const KEY& Key, const TYPE& Value)
{
	Node * TreeNode,*InsertPoint,*NewNode;
	TreeNode=m_Head;
	InsertPoint=NULL;

	//�ҳ�һ�������
	while(!TreeNode->IsSnil)
	{
		InsertPoint=TreeNode;
		if(Key==TreeNode->Key)			//��Key����
			return FALSE;

		if(_CMP()(Key,TreeNode->Key))
			TreeNode=TreeNode->LelfChild;
		else
			TreeNode=TreeNode->RightChild;
	}

	NewNode=new Node;
	if(NewNode==NULL)
		return FALSE;

	NewNode->Color=Node::RED;
	NewNode->Parent=InsertPoint;
	NewNode->LelfChild=&m_Snil;
	NewNode->RightChild=&m_Snil;
	NewNode->Value=Value;
	NewNode->Key=Key;
	NewNode->IsSnil=false;
	//�²���ڵ�Ϊ���ڵ�.
	if(InsertPoint==NULL) 
		m_Head=NewNode;
	else
	{
  		if(_CMP()(Key, InsertPoint->Key))
  			InsertPoint->LelfChild=NewNode;
  		else
  			InsertPoint->RightChild=NewNode;
	}
	//�Ըղ����½ڵ�ĺ��������ƽ�����.
	while(NewNode!=m_Head&&NewNode->Parent->Color==Node::RED)
	{   //������Ľڵ�ĸ��ڵ�Ϊүү�ڵ�����.
		if(NewNode->Parent==NewNode->Parent->Parent->LelfChild)
		{
			InsertPoint=NewNode->Parent->Parent->RightChild;
			if(InsertPoint->Color==Node::RED) 
			{
				NewNode->Parent->Color=Node::BLACK;
				NewNode->Parent->Parent->Color=Node::RED;
				InsertPoint->Color=Node::BLACK;
				NewNode=NewNode->Parent->Parent;
			}
			else
			{
				if(NewNode==NewNode->Parent->RightChild)
				{
					NewNode=NewNode->Parent;
					LeftRotate(NewNode);
				}
				NewNode->Parent->Color=Node::BLACK;
				NewNode->Parent->Parent->Color=Node::RED;
				RightRotate(NewNode->Parent->Parent);
			}
		}
		else
		{//������Ľڵ�ĸ��ڵ�Ϊүү�ڵ���ұ�.
			InsertPoint=NewNode->Parent->Parent->LelfChild;
			if(InsertPoint->Color==Node::RED)
			{
				NewNode->Parent->Color=Node::BLACK;
				NewNode->Parent->Parent->Color=Node::RED;
				InsertPoint->Color=Node::BLACK;
				NewNode=NewNode->Parent->Parent;
			}
			else
			{
				if(NewNode==NewNode->Parent->LelfChild)
				{
					NewNode=NewNode->Parent;
					RightRotate(NewNode);
				}
				NewNode->Parent->Color=Node::BLACK;
				NewNode->Parent->Parent->Color=Node::RED;
				LeftRotate(NewNode->Parent->Parent);
			}
		}
	}//end while ���к��������ƽ�����
	m_Count++;
	m_Head->Color=Node::BLACK; //�����Ϊ��
	return TRUE;
}

template<class KEY, class TYPE, class _CMP> 
CKMap<KEY,TYPE,_CMP>::CKMap()
{
	m_Snil.IsSnil=TRUE;
	m_Snil.LelfChild=NULL;
	m_Snil.RightChild=NULL;
	m_Snil.Color=Node::BLACK;
	m_Head=&m_Snil;m_Count=0;
};

template<class KEY, class TYPE, class _CMP> 
CKMap<KEY,TYPE,_CMP>::CKMap(const CKMap<KEY,TYPE,_CMP>& t)
{
	m_Snil.IsSnil		=TRUE;
	m_Snil.LelfChild	=NULL;
	m_Snil.RightChild	=NULL;
	m_Snil.Color		=Node::BLACK;
	m_Head				=&m_Snil;
	m_Count				=0;

	*this = t;
}

template<class KEY, class TYPE, class _CMP> 
CKMap<KEY,TYPE,_CMP>::~CKMap()
{
	Clear();
};

template<class KEY, class TYPE, class _CMP> 
BOOL CKMap<KEY,TYPE,_CMP>::Remove(const KEY& Key)
{
	Node * DeleteNode,*TreeNode,*R;
	DeleteNode=m_Head;
	//��Ҫɾ���Ľڵ�
	while(!DeleteNode->IsSnil&&DeleteNode->Key!=Key)
	{
		if(Key<DeleteNode->Key)
			DeleteNode=DeleteNode->LelfChild;
		else
			DeleteNode=DeleteNode->RightChild;
	}
	//��û���ҵ�Ҫɾ���Ľڵ�.
	if(DeleteNode->IsSnil)
		return FALSE;

	if(DeleteNode->LelfChild->IsSnil||DeleteNode->RightChild->IsSnil)
		TreeNode=DeleteNode;
	else
	{	//Ҫɾ���Ľڵ��������ӽڵ�.���Ծ��ҳ����ĺ����ڵ�.
		TreeNode=DeleteNode->RightChild;
		while(!TreeNode->LelfChild->IsSnil)
			TreeNode=TreeNode->LelfChild;
	}

	if(!TreeNode->LelfChild->IsSnil)//RΪɾ���ڵ���¸��ڵ�.
		R=TreeNode->LelfChild;
	else
		R=TreeNode->RightChild;

	R->Parent=TreeNode->Parent;

	if(TreeNode->Parent==NULL)
		m_Head=R;//ɾ�����������ڵ�
	else
	{
		if(TreeNode==TreeNode->Parent->LelfChild)
			TreeNode->Parent->LelfChild=R;
		else
			TreeNode->Parent->RightChild=R;
	}

	if(TreeNode!=DeleteNode)
	{
		DeleteNode->Value=TreeNode->Value;
		DeleteNode->Key=TreeNode->Key;
	}

	if(TreeNode->Color==Node::BLACK)
		DeleteFixUp(R);

	delete TreeNode;
	m_Count--;
	return TRUE;
}


template<class KEY, class TYPE, class _CMP> 
VOID CKMap<KEY,TYPE,_CMP>::DeleteTree(Node *TreeNode)
{
	if(TreeNode->IsSnil)
		return ;
	if(!TreeNode->LelfChild->IsSnil)
		DeleteTree(TreeNode->LelfChild);

	if(!TreeNode->RightChild->IsSnil)
		DeleteTree(TreeNode->RightChild);

	m_Count--;
	delete TreeNode;
}

template<class KEY, class TYPE, class _CMP> 
VOID CKMap<KEY,TYPE,_CMP>::LeftRotate(Node * TreeNode)
{
	Node * Rotate_Right;
	Rotate_Right=TreeNode->RightChild;
	TreeNode->RightChild=Rotate_Right->LelfChild;

	if(!Rotate_Right->LelfChild->IsSnil)
		Rotate_Right->LelfChild->Parent=TreeNode;

	Rotate_Right->Parent=TreeNode->Parent;
	if(TreeNode->Parent==NULL) //���������Ǹ��ڵ�.
		m_Head=Rotate_Right;
	else
		if(TreeNode->Parent->LelfChild==TreeNode)//�����Ľڵ�������һ���ڵ����
			TreeNode->Parent->LelfChild=Rotate_Right;
		else
			TreeNode->Parent->RightChild=Rotate_Right;

	Rotate_Right->LelfChild=TreeNode;
	TreeNode->Parent=Rotate_Right;
}

template<class KEY, class TYPE, class _CMP> 
VOID CKMap<KEY,TYPE,_CMP>::RightRotate(Node * TreeNode)
{
	Node * Rotate_Left;
	Rotate_Left=TreeNode->LelfChild;
	TreeNode->LelfChild=Rotate_Left->RightChild;
	if(!Rotate_Left->RightChild->IsSnil)
		Rotate_Left->RightChild->Parent=TreeNode;

	Rotate_Left->Parent=TreeNode->Parent;
	if(TreeNode->Parent==NULL)//Ϊ���ڵ�
		m_Head=Rotate_Left;
	else
		if(TreeNode->Parent->RightChild==TreeNode)
			TreeNode->Parent->RightChild=Rotate_Left;
		else
			TreeNode->Parent->LelfChild=Rotate_Left;

	Rotate_Left->RightChild=TreeNode;
	TreeNode->Parent=Rotate_Left;
}

template<class KEY, class TYPE, class _CMP> 
VOID CKMap<KEY,TYPE,_CMP>::DeleteFixUp(Node * TreeNode)
{
	Node * Brother;
	while(TreeNode!=m_Head&&TreeNode->Color==Node::BLACK)
	{
		if(TreeNode==TreeNode->Parent->LelfChild)
		{//��ǰ�ڵ��Ǹ��ڵ������
			Brother=TreeNode->Parent->RightChild;

			if(Brother->Color==Node::RED) 
			{//CASE 1
				Brother->Color=Node::BLACK;
				TreeNode->Parent->Color=Node::RED;
				LeftRotate(TreeNode->Parent);
				Brother=TreeNode->Parent->RightChild;
			}

			if(Brother->LelfChild->Color==Node::BLACK&&Brother->RightChild->Color==Node::BLACK)
			{//CASE 2
				Brother->Color=Node::RED;
				TreeNode=TreeNode->Parent;
			}
			else
			{
				if(Brother->RightChild->Color==Node::BLACK)
				{//CASE 3
					Brother->LelfChild->Color=Node::BLACK;
					Brother->Color=Node::RED;
					RightRotate(Brother);
					Brother=TreeNode->Parent->RightChild;
				}
				//CASE 4
				Brother->Color=TreeNode->Parent->Color;
				TreeNode->Parent->Color=Node::BLACK;
				Brother->RightChild->Color=Node::BLACK;
				LeftRotate(TreeNode->Parent);
				TreeNode=m_Head;	//�˳�
			}
			// end TreeNode==TreeNode->Parent->LelfChild
		}
		else			//�Գ�
		{//��ǰ�ڵ��Ǹ��ڵ���Һ���
			Brother=TreeNode->Parent->LelfChild;
			if (Brother->Color==Node::RED)
			{//CASE 1
				Brother->Color=Node::BLACK;
				TreeNode->Parent->Color=Node::RED;
				RightRotate(TreeNode->Parent);
				Brother=TreeNode->Parent->LelfChild;
			}
			if(Brother->LelfChild->Color==Node::BLACK&&Brother->RightChild->Color==Node::BLACK)
			{//CASE 2
				Brother->Color=Node::RED;
				TreeNode=TreeNode->Parent;
			}
			else
			{
				if(Brother->LelfChild->Color==Node::BLACK)
				{//CASE 3
					Brother->RightChild->Color=Node::BLACK;
					Brother->Color=Node::RED;
					LeftRotate(Brother);
					Brother=TreeNode->Parent->LelfChild;
				}
				//CASE 4
				Brother->Color=TreeNode->Parent->Color;
				TreeNode->Parent->Color=Node::BLACK;
				Brother->LelfChild->Color=Node::BLACK;
				RightRotate(TreeNode->Parent);
				TreeNode=m_Head;	//�˳�
			}
		}
	}//end while ����ƽ����

	TreeNode->Color=Node::BLACK;
}

template<class KEY, class TYPE, class _CMP> 
VOID CKMap<KEY,TYPE,_CMP>::Clear()
{
	DeleteTree(m_Head);
	m_Head=&m_Snil;
	m_Count=0;
}

template<class KEY, class TYPE, class _CMP> 
inline UINT CKMap<KEY,TYPE,_CMP>::Count()
{
	return m_Count;
}

template<class KEY, class TYPE, class _CMP> 
CKMap<KEY,TYPE,_CMP>& CKMap<KEY,TYPE,_CMP>::operator= (const CKMap<KEY,TYPE,_CMP>& t)
{
	Clear();
	CKMap<KEY,TYPE,_CMP>::Iterator it = t.Min();
	for (; it; it++)
	{
		Insert(KEY(it->Key),TYPE(it->Value));
	}

	return *this;
}


};//namespace msdk{
