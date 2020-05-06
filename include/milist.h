#pragma once


template <typename TNode>
class milist
{
public:
	typedef struct milist_node_t
	{
		struct  milist_node_t * prev, * next;
		TNode   data;
	}milist_node_t;

	typedef GenericIterator<milist_node_t, TNode, MoveNext<milist_node_t> > iterator;
	typedef GenericIterator<milist_node_t, TNode, MoveNext<milist_node_t> > const_iterator;
	typedef GenericIterator<milist_node_t, TNode, MovePrev<milist_node_t> > reverse_iterator;
	typedef GenericIterator<milist_node_t, TNode, MovePrev<milist_node_t> > const_reverse_iterator;
	iterator begin() const { return iterator(m_begin); }
	iterator end()   const { return iterator(0);       }
	reverse_iterator rbegin() const { return reverse_iterator(m_end); }
	reverse_iterator rend()   const { return reverse_iterator(0); }


public:
	milist()
		: m_begin(0)
		, m_end(0)
		, m_count(0)
	{}

	milist(const milist & mstr)
	{
		m_count = 0;
		m_begin = m_end = 0;
		copy_mlist(mstr);
	}

	~milist()
	{
		clear();
	}

	milist & operator = (const milist & mlst)
	{
		if(this == &mlst) return *this;

		copy_mlist(mlst);
		return *this;
	}

	bool empty() const
	{
		if(m_count == 0) { miassert(m_begin == 0 && m_end == 0); };
		return m_count == 0;
	}

	int  size() const
	{
		return m_count;
	}

	void clear()
	{
		if(m_count == 0)
		{
			miassert(m_begin == 0);
			miassert(m_end == 0);
			return;
		}

		milist_node_t * tmp = m_begin, * del = 0;
		while(tmp)
		{
			del = tmp;  tmp = tmp->next;
			destroy_list_node(del);
		}
		m_begin = m_end = 0; m_count = 0;
	}

	template <typename TNodeData>
	int  push_back(const TNodeData & data)
	{
		milist_node_t * node = 0;
		TNode           ndata(data);

		node = create_list_node(ndata);
		if(!node) return -1;

		node->next = node->next = 0;
		node->prev = m_end;

		if(!m_begin) m_begin = node;
		if(m_end)    m_end->next = node;
		m_end      = node; m_count++;
		return 0;
	}

	template <typename TNodeData>
	int  push_front(const TNodeData & data)
	{
		milist_node_t * node = 0;
		TNode           ndata(data);

		node = create_list_node(ndata);
		if(!node) return -1;

		node->next = node->next = 0;
		node->data = data;
		node->next = m_begin;

		if(!m_end)   m_end = node;
		if(m_begin)  m_begin->prev = node;
		m_begin    = node; m_count++;
		return 0;
	}

	void pop_back()
	{
		if(m_count > 0) delete_node(m_end);
	}

	void pop_front()
	{
		if(m_count > 0) delete_node(m_begin);
	}

	TNode & back()
	{
		miassert(m_count > 0);
		miassert(m_end);

		return m_end->data;
	}

	TNode & front()
	{
		miassert(m_count > 0);
		miassert(m_begin);

		return m_begin->data;
	}

	const TNode & back() const
	{
		miassert(m_count > 0);
		miassert(m_end);

		return m_end->data;
	}

	const TNode & front() const
	{
		miassert(m_count > 0);
		miassert(m_begin);

		return m_begin->data;
	}

	void swap(milist & mlst)
	{
		milist tmp;
		if(this == &mlst) return;

		memcpy(&tmp, &mlst, sizeof(*this));
		memcpy(&mlst, this, sizeof(*this));
		memcpy(this,  &tmp, sizeof(*this));
		memset(&tmp,  0,    sizeof(*this));
	}

	iterator erase(const iterator & it)
	{
		milist_node_t * node = it.get();

		miassert(node != 0);
		miassert(m_count > 0 && m_begin && m_end);
		if(node == 0)   return end();

		node = delete_node(node);
		return iterator(node);
	}

	void splice(const iterator off, milist & lst, const iterator lst_begin, const iterator lst_end)
	{
		// if(lst_end < lst_begin) ???
		milist_node_t * node = off.get(), * next = 0;
		milist_node_t * lst_node_begin = lst_begin.get(), * lst_node_end = lst_end.get();
		milist_node_t * lst_node_tmp = lst_node_begin;
		int             count = 1;

		if(lst_begin.index() > lst_end.index())
		{
			miassert(0);
			return ;
		}

		if(lst_node_begin == 0 || lst_node_begin == lst_node_end)
		{
			if(lst_node_begin == 0) miassert(lst.empty());
			return ;
		}

		node         = node ? node->prev : m_end;
		lst_node_end = lst_node_end ? lst_node_end->prev : lst.m_end;
		if(lst_node_end == 0)    lst_node_end  = lst.m_end;
		while(lst_node_tmp  !=   lst_node_end) { lst_node_tmp = lst_node_tmp->next; count++; }

		// delete from lst.
		if(lst_node_begin->prev) lst_node_begin->prev->next = lst_node_end->next;
		if(lst_node_end->next)   lst_node_end->next->prev   = lst_node_begin->prev;
		if(lst.m_begin == lst_node_begin) lst.m_begin = lst_node_end->next;
		if(lst.m_end   == lst_node_end)   lst.m_end   = lst_node_begin->prev;
		if(lst.m_count == count)          lst.m_begin = lst.m_end = 0;
		lst.m_count -= count;

		// insert into this.
		if(node == 0)
		{
			if(m_begin) m_begin->prev = lst_node_end;
			if(!m_end)  m_end         = lst_node_end;
			lst_node_end->next = m_begin; m_begin = lst_node_begin; m_count += count;
			return ;
		}

		next = node->next; node->next = lst_node_begin;
		lst_node_begin->prev = node; lst_node_end->next = next;
		if(next)  next->prev = lst_node_end;
		if(node == m_end) m_end = lst_node_end;
		m_count += count;
	}

	void splice(const iterator off, milist & lst)
	{
		return splice(off, lst, lst.begin(), lst.end());
	}

	// return node->next.
	milist_node_t * delete_node(milist_node_t * node)
	{
		miassert(node);

		milist_node_t * next = node->next;
		if(node->prev)      node->prev->next = node->next;
		if(node->next)      node->next->prev = node->prev;
		if(node == m_begin) m_begin = node->next;
		if(node == m_end)   m_end   = node->prev;

		destroy_list_node(node); m_count--;
		return next;
	}

	milist_node_t * back_node()
	{
		return m_end;
	}


private:
	void destroy_list_node(milist_node_t * node)
	{
		miassert(node);
		midestroy_node(&node->data);
		mifree(node);
	}

	milist_node_t * create_list_node(const TNode & node)
	{
		TNode         * ptr = 0;
		milist_node_t * list_node = (milist_node_t*)mimalloc(sizeof(milist_node_t));
		if(!list_node)  return 0;

		memset(list_node, 0, sizeof(*list_node));
		ptr = new(&list_node->data) TNode(node);
		if(!ptr)
		{
			mifree(list_node);
			return 0;
		}

		list_node->prev = list_node->next = 0;
		return list_node;
	}

	void copy_mlist(const milist & mlst)
	{
		clear();
		for(const_iterator it = mlst.begin(); it != mlst.end(); it++) push_back(*it);
	}


private:
	int  m_count;
	milist_node_t * m_begin, * m_end;
};


template <typename TNode>
inline typename milist<TNode>::milist_node_t * milist_data_to_node(TNode * data)
{
	return (milist<TNode>::milist_node_t*)((char*)data - offsetof(milist<TNode>::milist_node_t, data));
}
