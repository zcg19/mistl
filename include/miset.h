#pragma once


// ”Î misetœ‡À∆. 
template <typename TNode>
class miset
{
public:
	typedef struct miset_node_t
	{
		struct  miset_node_t * prev, * next;
		TNode   data;
	}miset_node_t;

	typedef GenericIterator<miset_node_t, TNode, MoveNext<miset_node_t> > iterator;
	typedef const GenericIterator<miset_node_t, TNode, MoveNext<miset_node_t> > const_iterator;
	iterator begin() const { return iterator(m_begin); }
	iterator end()   const { return iterator(0);       }


public:
	miset()
		: m_begin(0)
		, m_end(0)
		, m_count(0)
	{}

	miset(const miset & mstr)
	{
		m_count = 0;
		m_begin = m_end = 0;
		copy_mset(mstr);
	}

	~miset()
	{
		clear();
	}

	miset & operator = (const miset & mset)
	{
		if(this == &mset) return *this;

		copy_mset(mset);
		return *this;
	}

	bool empty()
	{
		if(m_count == 0) { miassert(m_begin == 0 && m_end == 0); };
		return m_count == 0;
	}

	int  size()
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

		miset_node_t * tmp = m_begin, * del = 0;
		while(tmp)
		{
			del =  tmp; tmp = tmp->next;
			destroy_list_node(del);
		}
		m_begin = m_end = 0; m_count = 0;
	}

	void swap(miset & mset)
	{
		miset tmp;
		if(this == &mset) return;

		memcpy(&tmp, &mset, sizeof(*this));
		memcpy(&mset, this, sizeof(*this));
		memcpy(this,  &tmp, sizeof(*this));
		memset(&tmp,  0,    sizeof(*this));
	}

	iterator erase(const iterator & it)
	{
		miset_node_t * node = it.get();

		miassert(node != 0);
		miassert(m_count > 0 && m_begin && m_end);
		if(node == 0)   return end();

		node = delete_node(node);
		return iterator(node);
	}

	iterator find(const TNode & data)
	{
		miset_node_t * it_node = 0;
		for(it_node = m_begin; it_node; it_node = it_node->next)
		{
			if(data == it_node->data) return iterator(it_node);
		}

		return end();
	}

	int  insert(const TNode & data)
	{
		miset_node_t * new_node = 0, * it_node = 0;

		for(it_node = m_begin; it_node; it_node = it_node->next)
		{
			if(data <  it_node->data) break;
		}

		if(it_node && it_node->prev && it_node->prev->data == data) return 1;
		if(!(new_node = create_set_node(data))) return -1;

		new_node->next = it_node;
		new_node->prev = it_node ? it_node->prev : m_end;

		if(it_node)
		{
			if(it_node->prev)  it_node->prev->next = new_node;
			it_node->prev = new_node;
		}
		else
		{
			if(m_end) m_end->next = new_node;
			m_end   = new_node;
		}

		if(it_node == m_begin)
		{ 
			if(m_begin) m_begin->prev = new_node;
			m_begin = new_node;
		}

		m_count++;
		return 0;
	}

	//
	miset_node_t * delete_node(miset_node_t * node)
	{
		miassert(node);

		miset_node_t * next = node->next;
		if(node->prev)      node->prev->next = node->next;
		if(node->next)      node->next->prev = node->prev;
		if(node == m_begin) m_begin = node->next;
		if(node == m_end)   m_end   = node->prev;

		destroy_list_node(node); m_count--;
		return next;
	}

	miset_node_t * back_node()
	{
		return m_end;
	}


private:
	void destroy_list_node(miset_node_t * node)
	{
		miassert(node);
		midestroy_node(&node->data);
		mifree(node);
	}

	miset_node_t * create_set_node(const TNode & node)
	{
		TNode        * ptr = 0;
		miset_node_t * set_node = (miset_node_t*)mimalloc(sizeof(miset_node_t));
		if(!set_node)  return 0;

		memset(set_node, 0, sizeof(*set_node));
		ptr = new(&set_node->data) TNode(node);
		if(!ptr)
		{
			mifree(set_node);
			return 0;
		}

		set_node->prev = set_node->next = 0;
		return set_node;
	}

	void copy_mset(const miset & mset)
	{
		clear();
		for(const_iterator it = mset.begin(); it != mset.end(); it++) push_back(*it);
	}


private:
	int  m_count;
	miset_node_t * m_begin, * m_end;
};
