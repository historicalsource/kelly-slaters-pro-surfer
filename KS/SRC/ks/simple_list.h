#ifndef SIMPLELIST_H
#define SIMPLELIST_H

// doubly-linked-list template class
// semi-tested, but be careful

template <class T>
class Node
{
public:
  T* Obj;
  Node<T> *next, *prev;

  Node() { Obj = NULL; next = prev = NULL; }
};

template <class T>
class List
{
private:
  Node<T> *head, *tail;
public:
  List()
  {
	  head = NEW Node<T>();
	  tail = NEW Node<T>();
	  head->next = tail;
	  tail->prev = head;
	  head->prev = tail->next = NULL;
  }

  ~List()
  {
	  DestroyList();
	  delete head;
	  delete tail;
  }

  Node<T>* GetFirst() const { return head; }
  Node<T>* GetLast() const { return tail; }

  void AddFront(T* Obj)
  {
	  Node<T> *node = NEW Node<T>();
	  node->Obj = Obj;

	  node->prev = head;
	  node->next = head->next;
	  head->next->prev = node;
	  head->next = node;
  }

  void AddBack(T* Obj)
  {
	 Node<T>* node = NEW Node<T>();
	 node->Obj = Obj;

     node->next = tail;
     node->prev = tail->prev;
     tail->prev->next = node;
     tail->prev = node;
  }

  void Remove(T* Obj)
  {
	for (Node<T>* node = head; node->next != NULL; node = node->next)
	{
		if (node->Obj == Obj)
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;
			delete node;
		}
	}
  }

  void Remove(Node<T>* node)
  {
	 node->prev->next = node->next;
	 node->next->prev = node->prev;
	 delete node;
  }

  void DestroyList(void)
  {
	  Node<T>* node = head->next;
	  while (node != tail)
	  {
		  Node<T>* temp = node->next;
		  delete node->Obj;
		  delete node;
		  node = temp;
	  }

	  head->next = tail;
	  tail->prev = head;
	  head->prev = tail->next = NULL;
  }

};


//When you want to add some object to a list, then the object must
//be defined like this:

/*
struct Object : public Node<Object>
{
};
*/

#endif // SIMPLELIST_H
