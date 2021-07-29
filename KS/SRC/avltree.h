#ifndef _TREE_HPP_
#define _TREE_HPP_

#ifndef TARGET_GC
#ifndef ARCH_ENGINE
#undef max
#include <stdlib.h>
#endif
#endif

/*

  Height balanced binary search tree class (AVL BST)

  by Jason Bare -- templatized by Greg Taylor

    the <T> class needs to have a public method  int compare(T *data)
    in order for this to build correctly.  It should return values like strcmp does.
*/
template <class T>
class AvlTree;

template <class T>
class TreeNode
{
protected:
  friend class AvlTree<T>;

	TreeNode *l;
	TreeNode *r;
	TreeNode *p;

	T *d;
	char h;

	TreeNode(T *data)
  {
  	l = r = p = NULL;
  	d = data;
  	h = 0;
  }
	~TreeNode()
  {
  	l = r = p = NULL;
  	d = NULL;
  	h = 0;
  }


	int compare(TreeNode *node) { return(compare(node->d)); }
	int compare(T *data)   { if(d != NULL) return(d->compare(data)); else return(-1); }

public:
	TreeNode *parent()		{ return (p); }
	TreeNode *left()		{ return (l); }
	TreeNode *right()		{ return (r); }
	T *data()			{ return (d); }

	char height()			{ return(h); }
};

template <class T>
class AvlTree
{
protected:
	typedef TreeNode<T> TNode;

	TNode *rootNode;

	int s;

	char disposeElements;

	void addHelper(TNode *node, TNode * &top, TNode *parent)
  {
	  if(top == NULL)
	  {
		  node->p = parent;
		  top = node;

		  s++;

		  return;
	  }

	  int comp = node->compare(top);

	  if(comp < 0)
	  {
		  addHelper(node, top->l, top);
		  if(nodeHt(top->l) - nodeHt(top->r) == 2)
		  {
			  if(node->compare(top->l) < 0)//node->ID < top->left->ID)
				  singleRotateLeft(top);
			  else
				  doubleRotateLeft(top);
		  }
		  else
			  calcHeight(top);
	  }
	  else if(comp > 0)
	  {
		  addHelper(node, top->r, top);

		  if(nodeHt(top->r) - nodeHt(top->l) == 2)
		  {
			  if(node->compare(top->r) > 0)//node->ID > top->right->ID)
				  singleRotateRight(top);
			  else
				  doubleRotateRight(top);
		  }
		  else
			  calcHeight(top);
	  }
  }

	TNode *findHelper(TNode *node, T *data)
  {
	  if(node != NULL && data != NULL)
	  {
		  int comp = -node->compare(data); // negative because of compare order

		  if(comp == 0)
			  return(node);
		  else if(comp < 0)
			  return(findHelper(node->l, data));
		  else if(comp > 0)
			  return(findHelper(node->r, data));
	  }

	  return(NULL);
  }

	void singleRotateLeft(TNode * &k2)
  {
	  TNode *k1 = k2->l;

	  k2->l = k1->r;
	  k1->r = k2;

	  k2->h = max(nodeHt(k2->l), nodeHt(k2->r)) + 1;
	  k1->h = max(nodeHt(k1->l), k2->h) + 1;

	  k2 = k1;
  }


	void doubleRotateLeft(TNode * &k3)
  {
  	singleRotateRight(k3->l);
  	singleRotateLeft(k3);
  }

	void singleRotateRight(TNode * &k2)
  {
	  TNode *k1 = k2->r;

	  k2->r = k1->l;
	  k1->l = k2;

	  k2->h = max(nodeHt(k2->l), nodeHt(k2->r)) + 1;
	  k1->h = max(nodeHt(k1->r), k2->h) + 1;

	  k2 = k1;
  }

	void doubleRotateRight(TNode * &k3)
  {
  	singleRotateLeft(k3->r);
  	singleRotateRight(k3);
  }

	int height(TNode *t)
  {
	  if(t==NULL)
		  return(-1);
	  else
		  return(1 + max(height(t->l), height(t->r)));
  }

	char nodeHt(TNode *node)
  {
  	return( node ? node->h: -1);
  }

	void calcHeight(TNode *node)
  {
	  node->h = 1 + max(nodeHt(node->l), nodeHt(node->r));
  }

	void dump(TNode * &node, int delData)
	//void dump(TreeNode * &node, int delData)
  {
  	if(node != NULL)
  	{
  		dump(node->l, delData);
  		dump(node->r, delData);

  		if(delData && node->d != NULL)
  			delete node->d;

  		delete node;
  		node = NULL;

  		s--;
  	}
  }

public:
	AvlTree(char disposeElem = 0)
  {
  	rootNode = NULL;

  	s = 0;

  	disposeElements = disposeElem;
  }

	~AvlTree()
  {
  	if(disposeElements)
  		dispose();
  	else
  		release();
  }

	void dispose() {  dump(rootNode, 1);  }

	void release() { 	dump(rootNode, 0);  }

	void add(T *data)
  {
  	if(data != NULL)
  	{
  		TNode *node = new TNode(data);
  		addHelper(node, rootNode, NULL);
  	}
  }

	TNode *find(T *data)
  {
  	return(findHelper(rootNode, data));
  }

	T *findData(T *data)
  {
  	TNode *node = findHelper(rootNode, data);
  	if(node != NULL)
  		return(node->d);
  	else
  		return(NULL);
  }

	TNode *root()						{ return (rootNode); }

	int size()								{ return (s); }
	int height()							{ return(height(rootNode)); }

	void setDisposeElements( char d )		{ disposeElements = d; }
};

#endif
