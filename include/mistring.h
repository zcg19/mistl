#pragma  once


template <typename TTchar>
misize_t mistrlen(const TTchar * str)
{
	int len = 0;
	while(*str++) len++;
	return len;
}

template <typename TTchar>
int  mistrcmp(const TTchar * str1, const TTchar * str2)
{
	int len1 = mistrlen(str1), len2 = mistrlen(str2);

	if(len1 != len2) return len1 < len2 ? -1 : 1;
	for(int i = 0; i < len1; i++)
	{
		if(str1[i] == str2[i]) continue;
		return str1[i] < str2[i] ? -1 : 1;
	}

	return 0;
}

template <typename TTchar>
const TTchar * mistrstr(const TTchar * str, const TTchar * sub) { miassert(0); return 0; }
template <> inline const char * mistrstr<char>(const char * str, const char * sub) 
{
	return strstr(str, sub);
}

template <> inline const wchar_t * mistrstr<wchar_t>(const wchar_t * str, const wchar_t * sub)
{
	return wcsstr(str, sub);
}


template <typename TTchar>
class mitstring
{
public:
	static  const misize_t  npos = -1;
	typedef GenericIterator<TTchar, TTchar, MoveAdd<TTchar> > iterator;
	typedef const GenericIterator<TTchar, TTchar, MoveAdd<TTchar> > const_iterator;
	iterator begin() const { return iterator(m_str); }
	iterator end()   const { return iterator(m_str ? m_str+m_len : 0);  }


public:
	mitstring(const TTchar * str = 0)
		: m_str(0)
		, m_len(0)
	{
		assign(str);
	}

	mitstring(const mitstring & mstr)
		: m_str(0)
		, m_len(0)
	{
		assign(mstr.c_str());
	}

	~mitstring()
	{
		clear();
	}

	mitstring & operator = (const TTchar * str)
	{
		assign(str);
		return *this;
	}

	mitstring & operator = (const mitstring & mstr)
	{
		if(this == &mstr) return *this;

		assign(mstr.c_str());
		return *this;
	}

	mitstring & operator += (const TTchar & mstr)
	{
		if(this == &mstr) return *this;

		append(mstr, -1);
		return *this;
	}

	mitstring & operator += (const mitstring & mstr)
	{
		if(this == &mstr) return *this;

		append(mstr.c_str(), -1);
		return *this;
	}

	bool operator == (const mitstring & mstr) const
	{
		if(&mstr == this) return true;
		if(mstr.empty())  return empty();
		if(empty())       return mstr.empty();

		return mistrcmp(m_str, mstr.c_str()) == 0;
	}

	bool operator != (const mitstring & mstr) const
	{
		return !(mstr == *this);
	}

	TTchar & operator [](int off)
	{
		miassert(m_str);
		static TTchar  cdef = 0;
		return m_str ? m_str[off] : cdef;
	}

	int  length() const
	{
		return m_len;
	}

	int  size() const
	{
		return m_len;
	}

	bool empty() const
	{
		return m_len == 0;
	}

	int  clear()
	{
		if(m_str)
		{
			mifree(m_str);
			m_str = 0; m_len = 0;
		}

		return 0;
	}

	TTchar & at(misize_t i)
	{
		miassert(i < (misize_t)m_len);
		static TTchar cdef = 0;
		return i < (misize_t)m_len ? m_str[i] : cdef;
	}

	const TTchar * c_str() const 
	{
		static TTchar cdef = 0;
		return m_len ? m_str : &cdef;
	}

	misize_t find(TTchar ch, misize_t off = 0)
	{
		for(int i = 0; i < m_len; i++)
		{
			if(m_str[i] == ch) return i;
		}

		return npos;
	}

	misize_t find(const TTchar * str, misize_t off = 0)
	{
		const TTchar * p = 0;

		if(m_str)  p = mistrstr(m_str, str);
		return p ? p-m_str : npos;
	}

	int  resize(misize_t sz, const TTchar ch = 0)
	{
		TTchar * ptr = 0;

		if(sz == 0)
		{
			clear();
			return 0;
		}

		ptr = (TTchar*)mimalloc((sz+1)*sizeof(TTchar));
		if(!ptr) return -1;

		for(int i = 0; i < (int)sz; i++) ptr[i] = ch; ptr[sz] = 0;
		memcpy(ptr, m_str, m_len > (int)sz ? sz : m_len);
		clear(); m_str = ptr; m_len = sz;
		return 0;
	}

	int  replace(int off, int slen, const TTchar * str, int dlen = -1)
	{
		if(dlen == 0 || !str || *str == 0) return 0;
		if(slen <= 0 || slen > m_len || (off < 0 || off >= m_len)) return -2;

		int nlen = 0;
		if(dlen < 0) dlen = mistrlen(str); nlen = m_len+dlen-slen;
		if(dlen > slen)
		{
			TTchar * ptr = 0;
			ptr = (TTchar*)mimalloc((nlen+1)*sizeof(TTchar));
			if(!ptr) return -1;

			if(off > 0) mimemcpy(ptr, m_str, off*sizeof(TTchar));
			mimemcpy(ptr+off, str, dlen); miassert(dlen > 0);
			if(m_len-off-slen > 0) mimemcpy(ptr+off+dlen, m_str+off+slen, (m_len-off-slen)*sizeof(TTchar));
			ptr[nlen] = 0;

			clear(); m_str = ptr; m_len = nlen;
		}
		else
		{
			miassert(dlen+off <= m_len);
			mimemcpy(m_str+off, str, dlen);
			if(m_len-off-slen > 0) mimemmove(m_str+off+dlen, m_str+off+slen, (m_len-off-slen)*sizeof(TTchar));
			m_str[nlen] = 0; m_len = nlen;
		}

		return 0;
	}

	int  insert(int off, const TTchar * str, int len = -1)
	{
		TTchar * ptr = 0;
		if(len == 0 || !str || *str == 0) return 0;
		if(off > m_len) return -2;

		if(off < 0) off = m_len;
		if(len < 0) len = mistrlen(str);
		ptr = (TTchar*)mimalloc((len+m_len+1)*sizeof(TTchar));
		if(!ptr) return -1;

		if(off > 0) mimemcpy(ptr, m_str, off*sizeof(TTchar));
		mimemcpy(ptr+off, str, len*sizeof(TTchar));
		if(off < m_len) mimemcpy(ptr+off+len, m_str+off, (m_len-off)*sizeof(TTchar));
		ptr[m_len+len] = 0;

		len  += m_len; clear();
		m_str = ptr; m_len = len;
		return 0;
	}

	int  append(const TTchar * str, misize_t len = -1)
	{
		return insert(m_len, str, len);
	}

	int  assign(const TTchar * str, int len = -1)
	{
		clear();
		append(str, len);
		return 0;
	}

	void swap(mitstring & mstr)
	{
		mitstring tmp;
		if(&mstr == this) return ;

		memcpy(&tmp, &mstr, sizeof(*this));
		memcpy(&mstr, this, sizeof(*this));
		memcpy(this,  &tmp, sizeof(*this));
		memset(&tmp,  0,    sizeof(*this));
	}


private:
	int      m_len;
	TTchar * m_str;
};


typedef mitstring<char>    mistring;
typedef mitstring<wchar_t> miwstring;


template <typename TTchar>
inline bool operator == (const TTchar * str, const mitstring<TTchar> & mstr)
{
	if(mstr.empty()) return str == 0;
	if(!str) return mstr.empty();

	return mistrcmp(mstr.c_str(), str) == 0;
}

template <typename TTchar>
inline mitstring<TTchar> operator+(const mitstring<TTchar> & left, const TTchar * right)
{
	mitstring<TTchar> tmp = left; tmp.append(right);
	return tmp;
}

template <> inline int micalc_hash_node<mistring>(const mistring & s, minonscalar_ptr)
{
	return micalc_hash_string((miuchar*)s.c_str(), (int)s.length());
}

template <> inline int micalc_hash_node<miwstring>(const miwstring & s, minonscalar_ptr)
{
	return micalc_hash_string((miuchar*)s.c_str(), (int)s.length()*sizeof(wchar_t));
}
