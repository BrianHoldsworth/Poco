//
// AbstractContainerNode.cpp
//
// $Id: //poco/1.4/XML/src/AbstractContainerNode.cpp#1 $
//
// Library: XML
// Package: DOM
// Module:  DOM
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/DOM/AbstractContainerNode.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/DOMException.h"


namespace Poco {
namespace XML {


AbstractContainerNode::AbstractContainerNode(Document* pOwnerDocument): 
	AbstractNode(pOwnerDocument),
	_pFirstChild(0)
{
}


AbstractContainerNode::AbstractContainerNode(Document* pOwnerDocument, const AbstractContainerNode& node): 
	AbstractNode(pOwnerDocument, node),
	_pFirstChild(0)
{
}


AbstractContainerNode::~AbstractContainerNode()
{
	AbstractNode* pChild = static_cast<AbstractNode*>(_pFirstChild);
	while (pChild)
	{
		AbstractNode* pDelNode = pChild;
		pChild = pChild->_pNext;
		pDelNode->_pNext   = 0;
		pDelNode->_pParent = 0;
		pDelNode->release();
	}
}


Node* AbstractContainerNode::firstChild() const
{
	return _pFirstChild;
}


Node* AbstractContainerNode::lastChild() const
{
	AbstractNode* pChild = _pFirstChild;
	if (pChild)
	{
		while (pChild->_pNext) pChild = pChild->_pNext;
		return pChild;
	}
	return 0;
}


Node* AbstractContainerNode::insertBefore(Node* newChild, Node* refChild)
{
	poco_check_ptr (newChild);

	if (static_cast<AbstractNode*>(newChild)->_pOwner != _pOwner && static_cast<AbstractNode*>(newChild)->_pOwner != this)
		throw DOMException(DOMException::WRONG_DOCUMENT_ERR);
	if (refChild && static_cast<AbstractNode*>(refChild)->_pParent != this)
		throw DOMException(DOMException::NOT_FOUND_ERR);
	if (newChild == refChild)
		return newChild;
	if (this == newChild)
		throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);

	AbstractNode* pFirst = 0;
	AbstractNode* pLast  = 0;
	if (newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE)
	{
		AbstractContainerNode* pFrag = static_cast<AbstractContainerNode*>(newChild);
		pFirst = pFrag->_pFirstChild;
		pLast  = pFirst;
		if (pFirst)
		{
			while (pLast->_pNext)
			{
				pLast->_pParent = this;
				pLast = pLast->_pNext;
			}
			pLast->_pParent = this;
		}
		pFrag->_pFirstChild = 0;
	}
	else
	{
		newChild->duplicate();
		AbstractContainerNode* pParent = static_cast<AbstractNode*>(newChild)->_pParent;
		if (pParent) pParent->removeChild(newChild);
		pFirst = static_cast<AbstractNode*>(newChild);
		pLast  = pFirst;
		pFirst->_pParent = this;
	}
	if (_pFirstChild && pFirst)
	{
		AbstractNode* pCur = _pFirstChild;
		if (pCur == refChild)
		{
			pLast->_pNext = _pFirstChild;
			_pFirstChild  = pFirst;
		}
		else
		{
			while (pCur && pCur->_pNext != refChild) pCur = pCur->_pNext;
			if (pCur)
			{
				pLast->_pNext = pCur->_pNext;
				pCur->_pNext = pFirst;
			}
			else throw DOMException(DOMException::NOT_FOUND_ERR);
		}
	}
	else _pFirstChild = pFirst;

	if (events())
	{
		while (pFirst && pFirst != pLast->_pNext)
		{
			pFirst->dispatchNodeInserted();
			pFirst->dispatchNodeInsertedIntoDocument();
			pFirst = pFirst->_pNext;
		}
		dispatchSubtreeModified();
	}
	return newChild;
}


Node* AbstractContainerNode::replaceChild(Node* newChild, Node* oldChild)
{
	poco_check_ptr (newChild);
	poco_check_ptr (oldChild);

	if (static_cast<AbstractNode*>(newChild)->_pOwner != _pOwner && static_cast<AbstractNode*>(newChild)->_pOwner != this)
		throw DOMException(DOMException::WRONG_DOCUMENT_ERR);
	if (static_cast<AbstractNode*>(oldChild)->_pParent != this)
		throw DOMException(DOMException::NOT_FOUND_ERR);
	if (newChild == oldChild)
		return newChild;
	if (this == newChild)
		throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);

	bool doEvents = events();
	if (newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE)
	{
		insertBefore(newChild, oldChild);
		removeChild(oldChild);
	}
	else
	{
		AbstractContainerNode* pParent = static_cast<AbstractNode*>(newChild)->_pParent;
		if (pParent) pParent->removeChild(newChild);

		if (oldChild == _pFirstChild)
		{
			if (doEvents)
			{
				_pFirstChild->dispatchNodeRemoved();
				_pFirstChild->dispatchNodeRemovedFromDocument();
			}
			static_cast<AbstractNode*>(newChild)->_pNext   = static_cast<AbstractNode*>(oldChild)->_pNext;
			static_cast<AbstractNode*>(newChild)->_pParent = this;
			_pFirstChild->_pNext   = 0;
			_pFirstChild->_pParent = 0;
			_pFirstChild = static_cast<AbstractNode*>(newChild);
			if (doEvents)
			{
				static_cast<AbstractNode*>(newChild)->dispatchNodeInserted();
				static_cast<AbstractNode*>(newChild)->dispatchNodeInsertedIntoDocument();
			}
		}
		else
		{
			AbstractNode* pCur = _pFirstChild;
			while (pCur && pCur->_pNext != oldChild) pCur = pCur->_pNext;
			if (pCur)
			{	
				poco_assert_dbg (pCur->_pNext == oldChild);

				if (doEvents)
				{
					static_cast<AbstractNode*>(oldChild)->dispatchNodeRemoved();
					static_cast<AbstractNode*>(oldChild)->dispatchNodeRemovedFromDocument();
				}
				static_cast<AbstractNode*>(newChild)->_pNext   = static_cast<AbstractNode*>(oldChild)->_pNext;
				static_cast<AbstractNode*>(newChild)->_pParent = this;
				static_cast<AbstractNode*>(oldChild)->_pNext   = 0;
				static_cast<AbstractNode*>(oldChild)->_pParent = 0;
				pCur->_pNext = static_cast<AbstractNode*>(newChild);
				if (doEvents)
				{
					static_cast<AbstractNode*>(newChild)->dispatchNodeInserted();
					static_cast<AbstractNode*>(newChild)->dispatchNodeInsertedIntoDocument();
				}
			}
			else throw DOMException(DOMException::NOT_FOUND_ERR);
		}
		newChild->duplicate();
		oldChild->autoRelease();
	}
	if (doEvents) dispatchSubtreeModified();
	return oldChild;
}


Node* AbstractContainerNode::removeChild(Node* oldChild)
{
	poco_check_ptr (oldChild);

	bool doEvents = events();
	if (oldChild == _pFirstChild)
	{
		if (doEvents)
		{
			static_cast<AbstractNode*>(oldChild)->dispatchNodeRemoved();
			static_cast<AbstractNode*>(oldChild)->dispatchNodeRemovedFromDocument();
		}
		_pFirstChild = _pFirstChild->_pNext;
		static_cast<AbstractNode*>(oldChild)->_pNext   = 0;
		static_cast<AbstractNode*>(oldChild)->_pParent = 0;
	}
	else
	{
		AbstractNode* pCur = _pFirstChild;
		while (pCur && pCur->_pNext != oldChild) pCur = pCur->_pNext;
		if (pCur)
		{
			if (doEvents)
			{
				static_cast<AbstractNode*>(oldChild)->dispatchNodeRemoved();
				static_cast<AbstractNode*>(oldChild)->dispatchNodeRemovedFromDocument();
			}
			pCur->_pNext = pCur->_pNext->_pNext;
			static_cast<AbstractNode*>(oldChild)->_pNext   = 0;
			static_cast<AbstractNode*>(oldChild)->_pParent = 0;
		}
		else throw DOMException(DOMException::NOT_FOUND_ERR);
	}
	oldChild->autoRelease();
	if (doEvents) dispatchSubtreeModified();
	return oldChild;
}


Node* AbstractContainerNode::appendChild(Node* newChild)
{
	return insertBefore(newChild, 0);
}


void AbstractContainerNode::dispatchNodeRemovedFromDocument()
{
	AbstractNode::dispatchNodeRemovedFromDocument();
	Node* pChild = firstChild();
	while (pChild)
	{
		static_cast<AbstractNode*>(pChild)->dispatchNodeRemovedFromDocument();
		pChild = pChild->nextSibling();
	}
}


void AbstractContainerNode::dispatchNodeInsertedIntoDocument()
{
	AbstractNode::dispatchNodeInsertedIntoDocument();
	Node* pChild = firstChild();
	while (pChild)
	{
		static_cast<AbstractNode*>(pChild)->dispatchNodeInsertedIntoDocument();
		pChild = pChild->nextSibling();
	}
}


bool AbstractContainerNode::hasChildNodes() const
{
	return _pFirstChild != 0;
}


bool AbstractContainerNode::hasAttributes() const
{
	return false;
}


} } // namespace Poco::XML
