#pragma once


#include "milist.h"


#ifndef MIMAP_HASH_SIZE
#define MIMAP_HASH_SIZE    256
#endif


template <typename TKey, typename TValue>
class mimap
{
public:
	static  const  int hash_table_size = MIMAP_HASH_SIZE;
	typedef mipair<TKey, TValue>                           mimap_node_t;
	typedef typename milist<mimap_node_t>::milist_node_t * mimap_node_hash_t;
	typedef milist<mimap_node_hash_t>                      mimap_list_hash_t;
	typedef milist<mimap_node_t>                           mimap_list_node_t;
	typedef mimap_list_hash_t                            * mimap_table_hash_t[MIMAP_HASH_SIZE];

	typedef typename mimap_list_node_t::iterator iterator;
	typedef typename mimap_list_node_t::const_iterator const_iterator;
	iterator begin() const { return m_data_table.begin(); }
	iterator end()   const { return m_data_table.end();   }


public:
	mimap()
		: m_hash_table(0)
	{
		create_hash_table();
	}

	mimap(const mimap & mmap)
	{
		create_hash_table();
		for(iterator it = mmap.begin(); it != mmap.end(); it++)
		{
			insert(it->first, it->second);
		}
	}

	~mimap()
	{
		clear();
		if(m_hash_table)
		{
			for(int i = 0; i < hash_table_size; i++) 
			{
				if((*m_hash_table)[i]) 
				{
					midestroy_node((*m_hash_table)[i]);
					mifree((*m_hash_table)[i]);
				}
			}
			mifree(m_hash_table);
		}
	}

	int  size() const
	{
		return m_data_table.size();
	}

	bool empty() const
	{
		return m_data_table.empty();
	}

	void clear()
	{
		if(m_hash_table)
		{
			for(int i = 0; i < hash_table_size; i++)
			{
				if((*m_hash_table)[i]) { (*m_hash_table)[i]->clear(); }
			}
		}
		m_data_table.clear();
	}

	void swap(mimap & mmap)
	{
		if(this == &mmap) return ;

		mimap_table_hash_t * tmp = mmap.m_hash_table;
		mmap.m_hash_table  = m_hash_table;
		m_hash_table       = tmp;
		m_data_table.swap(mmap.m_data_table);
	}

	mimap & operator = (const mimap & mmap)
	{
		if(this == &mmap) return *this;

		clear();
		for(iterator it = mmap.begin(); it != mmap.end(); it++)
		{
			insert(it->first, it->second);
		}

		return *this;
	}

	template <typename TNodeKey>
	TValue & operator [] (const TNodeKey & ckey)
	{
		iterator it = find(ckey);
		if(it == end())
		{
			insert(ckey, TValue());
			it = find(ckey);
			miassert(it != end());
		}

		return it->second;
	}

	iterator erase(const iterator & it)
	{
		return erase(it->first);
	}

	template <typename TNodeKey>
	iterator erase(const TNodeKey & ckey)
	{
		mimap_node_hash_t h = 0;

		h = find(ckey, true);
		if(!h) return end();

		return m_data_table.erase(iterator(h));
	}

	template <typename TNodeKey, typename TNodeValue>
	int  insert(const  mipair<TNodeKey, TNodeValue> & cnode)
	{
		return insert(cnode.first, cnode.second);
	}

	template <typename TNodeKey, typename TNodeValue>
	int  insert(const TNodeKey & ckey, const TNodeValue & cvalue)
	{
		int  hash_key  = 0, r = 0;
		if(!m_hash_table) return -3;

		do
		{
			mimap_list_hash_t * hash_list, *ptr;
			mimap_node_t        node(ckey, cvalue);
			hash_key  = calc_hash(node.first);
			hash_list = (*m_hash_table)[hash_key];
			if(!hash_list)
			{
				if(!(hash_list = (mimap_list_hash_t*)mimalloc(sizeof(mimap_list_hash_t)))) { r = -4; break; }
				ptr = new(hash_list) mimap_list_hash_t;
				(*m_hash_table)[hash_key] = ptr;
			}
			else
			{
				for(mimap_list_hash_t::iterator it = hash_list->begin(); it != hash_list->end(); it++)
				{
					miassert(*it);
					mimap_node_t & tmp = (*it)->data;
					if(tmp.first == node.first) { r = 1; break; };
				}
			}

			if(r) break;
			if(m_data_table.push_back(node))  { r = -5; break; }
			if(hash_list->push_back(m_data_table.back_node())) { r = -6; break; }

			return 0;
		}while(0);

		return r;
	}

	template <typename TNodeKey>
	iterator find(const TNodeKey & ckey) const
	{
		mimap * pthis = (mimap*)this;
		return  pthis->find(ckey);
	}

	template <typename TNodeKey>
	iterator find(const TNodeKey & ckey)
	{
		mimap_node_hash_t h = 0;

		h = find(ckey, false);
		return iterator(h);
	}


private:
	int  create_hash_table()
	{
		m_hash_table = (mimap_table_hash_t*)mimalloc(sizeof(*m_hash_table));
		if(m_hash_table) memset(m_hash_table, 0, sizeof(*m_hash_table));
		return m_hash_table ? 0 : -1;
	}

	template <typename TNodeKey>
	mimap_node_hash_t find(const TNodeKey & ckey, bool del)
	{
		TKey key(ckey);
		mimap_list_hash_t * hash_list;

		if(!m_hash_table) return 0;
		if(hash_list = (*m_hash_table)[calc_hash(key)])
		{
			for(mimap_list_hash_t::iterator it = hash_list->begin(); it != hash_list->end(); it++)
			{
				miassert(*it);
				mimap_node_hash_t h = *it;
				mimap_node_t    & n = h->data;
				if(n.first == key)
				{
					if(del) hash_list->erase(it);
					return  h;
				}
			}
		}

		return 0;
	}

	int  calc_hash(const TKey & key)
	{
		miptr_type_helper<TKey>::miptr_type t;
		int h = micalc_hash_node(key, t);
		return h%hash_table_size;
	}


private:
	mimap_table_hash_t * m_hash_table;
	mimap_list_node_t    m_data_table;
};
