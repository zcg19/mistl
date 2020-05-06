#pragma once


template <typename TNode>
class mivector
{
public:
	typedef GenericIterator<TNode, TNode, MoveAdd<TNode> > iterator;
	typedef const GenericIterator<TNode, TNode, MoveAdd<TNode> > const_iterator;
	iterator begin() const { return iterator(m_node); }
	iterator end()   const { return iterator(m_count ? m_node+m_count : 0);  }


public:
	mivector()
		: m_node(0)
		, m_count(0)
		, m_size(0)
	{}

	mivector(const mivector & mvec)
		: m_node(0)
		, m_count(0)
		, m_size(0)
	{
		copy_mvector(mvec);
	}

	mivector(misize_t sz, const TNode n = TNode())
		: m_node(0)
		, m_count(0)
		, m_size(0)
	{
		resize(sz, n);
	}

	mivector & operator = (const mivector & mvec)
	{
		if(this == &mvec) return *this;

		copy_mvector(mvec);
		return *this;
	}

	bool operator == (const mivector & mvec) const
	{
		if(this == &mvec) return true;
		if(m_count != mvec.m_count) return false;
		for(int i = 0; i < m_count; i++) { if(m_node[i] != mvec.m_node[i]) return false; }
		return true;
	}

	bool operator != (const mivector & mvec) const
	{
		return !(mvec == *this);
	}

	TNode & operator [](int off)
	{
		miassert(m_count > 0);
		miassert(m_node);
		return m_node[off];
	}

	~mivector()
	{
		clear();
	}

	bool empty()
	{
		return m_count == 0;
	}

	int  size()
	{
		return m_count;
	}

	void clear()
	{
		if(m_node)
		{
			miassert(m_size > 0);
			for(int i = 0,  n = m_count; i < n; i++) destroy_node(&m_node[i]);
			mifree(m_node); m_node = 0; m_count = m_size = 0;
		}
	}

	void swap(mivector & mvec)
	{
		mivector tmp;
		if(&mvec == this) return ;

		memcpy(&tmp, &mvec, sizeof(*this));
		memcpy(&mvec, this, sizeof(*this));
		memcpy(this,  &tmp, sizeof(*this));
		memset(&tmp,  0,    sizeof(*this));
	}

	TNode & at(misize_t pos)
	{
		miassert(pos < m_count);
		return *(m_node+pos);
	}

	int  push_back(const TNode & node)
	{
		return insert_node(m_count, node);
	}

	int  pop_back()
	{
		if(m_count == 0) return 0;

		destroy_node(&m_node[m_count-1]);
		return 0;
	}

	TNode & back()
	{
		miassert(m_count > 0);
		miassert(m_node);
		return m_node[m_count-1];
	}

	TNode & front()
	{
		miassert(m_count > 0);
		miassert(m_node);
		return m_node[0];
	}

	int  resize(misize_t sz, const TNode node = TNode())
	{
		if(sz == 0) { clear(); return 0; }
		if(sz <= (misize_t)m_size) { m_count = m_size; return 0; }

		TNode * ptr = (TNode*)mimalloc(sizeof(TNode)*sz);
		if(!ptr) return -1;

		memset(ptr, 0, sizeof(*ptr)*sz);
		for(int i = 0; i < (int)sz; i++) ptr[i] = node;
		for(int i = 0, n = m_count > (int)sz ? (int)sz : m_count; i < n; i++) ptr[i] = m_node[i];
		clear(); m_count = m_size = sz;  m_node = ptr;
		return 0;
	}

	iterator insert(const iterator & it, const TNode & node)
	{
		int     index = it.index(), ret = 0;

		ret = insert_node(it.get() ? index : m_count, node);
		return ret ? end() : iterator(m_node+index);
	}

	iterator erase(const iterator & it)
	{
		int     find = -1;
		TNode * node = it.get();
		if(it == 0) return end();

		miassert(!empty());
		for(int i = 0; i < m_count; i++)
		{
			if(find < 0 && node == m_node + i)
			{
				find = i;
				destroy_node(node);
			}

			if(find >= 0 && i < m_count) m_node[i] = m_node[i+1];
		}

		miassert(find >= 0);
		return (find+1) >= m_count ? end() : iterator(m_node+find+1);
	}


private:
	void destroy_node(TNode * node)
	{
		midestroy_node(node);
		m_count--;
	}

	void copy_mvector(const mivector & mvec)
	{
		for(int i = 0; i < m_count; i++) destroy_node(&m_node[i]);
		for(const_iterator it = mvec.begin(); it != mvec.end(); it++) push_back(*it);
	}

	int  insert_node(int index, const TNode & node)
	{
		assert(index <= m_count);
		if(m_count < m_size)
		{
			for(int i = m_count; i > index; i--) m_node[i] = m_node[i-1];
			m_node[index] = node; m_count++;
			return 0;
		}

		int     size = m_size+4, count = m_count+1;
		TNode * new_node, def_node = TNode();
		miassert(m_count == m_size);
		new_node  = (TNode*)mimalloc(sizeof(TNode)*size);
		if(new_node == 0) return -1;

		memset(new_node, 0, sizeof(*new_node)*size);
		for(int i = 0; i < index; i++) new_node[i] = m_node[i];
		for(int i = index+1; i < count; i++) new_node[i] = m_node[i-1];
		new_node[index] = node;

		clear(); m_node = new_node; m_count = count; m_size = size;
		return 0;
	}


private:
	TNode * m_node;
	int     m_count, m_size;
};
