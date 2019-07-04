#pragma once


#pragma warning(disable:4996)

#include<iostream>
#include<stdio.h>
#include<string.h>
#define _CRT_SECURE_NO_WARNINGS
using namespace std;
typedef unsigned int size_t;

class String
{
public:
	typedef char* iterator;

public:
	String(const char* str = "")
	{
		if (str == nullptr)
			return;
		_size = strlen(str);
		_capacity = _size;
		_str = new char[_size + 1];
		strcpy(_str, str);
	}

	String(const String& str)
	{
		if (&str == this)
			return;
		_size = str._size;
		_capacity = str._capacity;
		_str = new char[str._capacity + 1];
		strcpy(_str, str._str);
	}

	String& operator=(const String& str)
	{
		if (&str == this)
			return *this;
		char* pstr = new char[str._capacity + 1];
		strcpy(pstr, str._str);
		if(_str)
			delete[] _str;
		_str = pstr;
		_size = str._size;
		_capacity = str._capacity;
		return *this;
	}

	~String()
	{
		if (_str)
		{
			delete[] _str;
			_str = nullptr;
		}
	}

	iterator Begin()
	{
		return _str;
	}

	iterator End()
	{
		return _str + _size;
	}


	void Push_Back(char c)
	{
		if (_size == _capacity)
			Reserve(2 * _capacity);
		_str[_size] = c;
		_size++;
		_str[_size] = '\0';
	}

	void Append(size_t n, char c)
	{
		for (int i = 0; i < n; i++)
		{
			Push_Back(c);
		}
	}

	String& operator+=(char c)
	{
		Push_Back(c);
		return *this;
	}

	void Append(const char* str)
	{
		int len = strlen(str);
		if (len + _size >= _capacity)
			Reserve(2 * _capacity);
		for (int i = 0; i < len; i++)
		{
			_str[_size + i] = str[i];
		}
		_size = _size + len;
		_str[_size] = '\0';
	}

	void Clear()
	{
		_size = 0;
		_capacity = 0;
		if (_str)
		{
			delete[] _str;
		}
		
	}

	void Swap(String& str)
	{
		char* pstr = str._str;
		str._str = _str;
		_str = pstr;

		int num = str._size;
		str._size = _size;
		_size = num;

		num = str._capacity;
		str._capacity = _capacity;
		_capacity = num;
	}

	const char* C_STR()const
	{
		return _str;
	}

	size_t Size()const
	{
		return _size;
	}

	size_t Capacity()const
	{
		return _capacity;
	}

	bool Empty()const
	{
		if (_str == nullptr)
			return true;
		return false;
	}

	void Resize(size_t newsize, char c = char())
	{
		if (newsize < _size)
		{
			_size = newsize;
			_str[_size] = '\0';
		}
		else if (newsize > _size)
		{
			if (newsize >= _capacity)
				Reserve(2 * _capacity);
			for (int i = _size; i < newsize; i++)
			{
				_str[i] = c;
			}
			_str[newsize] = '\0';
		}
	}

	void Reserve(size_t newCapacity)
	{
		if (newCapacity > _capacity)
		{
			char* pstr = new char[newCapacity + 1];
			strcpy(pstr, _str);
			if(_str)
				delete[] _str;
			_str = pstr;
			_capacity = newCapacity;
		}
	}


	char& operator[](size_t index)
	{
		return _str[index];
	}

	const char& operator[](size_t index)const
	{
		return _str[index];
	}

	bool operator<(const String& str)
	{
		if (strcmp(_str, str._str) < 0)
		{
			return true;
		}
		return false;
	}

	bool operator<=(const String& str)
	{
		if (strcmp(_str, str._str) <= 0)
		{
			return true;
		}
		return false;
	}

	bool operator>(const String& str)
	{
		if (strcmp(_str, str._str) > 0)
		{
			return true;
		}
		return false;
	}

	bool operator>=(const String& str)
	{
		if (strcmp(_str, str._str) >= 0)
		{
			return true;
		}
		return false;
	}

	bool operator==(const String& str)
	{
		if (strcmp(_str, str._str) == 0)
		{
			return true;
		}
		return false;
	}

	bool operator!=(const String& str)
	{
		if (strcmp(_str, str._str) != 0)
		{
			return true;
		}
		return false;
	}

	//µÚÒ»¸ö
	size_t Find(char c, size_t pos = 0)const
	{
		for (int i = pos; i < _size; i++)
		{
			if (_str[i] == c)
			{
				return i;
			}
		}
		return -1;
	}

	size_t Find(const char* s, size_t pos = 0)const
	{
		for (int i = pos; i < _size; i++)
		{
			if (strcmp(_str, s) == 0)
			{
				return pos;
			}
		}
		return -1;
	}

	String SubStr(size_t pos, size_t n)
	{
		char* pstr = new char[n + 1];
		for (int i = 0; i < n; i++)
		{
			pstr[i] = _str[pos + i];
		}
		pstr[n] = '\0';
		String str(pstr);
		return str;
	}

	String& Insert(size_t pos, char c)
	{
		if (_size + 1 >= _capacity)
			Reserve(_capacity * 2);
		for (int i = _size; i >= pos; i--)
		{
			_str[i + 1] = _str[i];
		}
		_str[pos] = c;
		_size += 1;
		return *this;
	}

	String& Insert(size_t pos, const char* str)
	{
		int len = strlen(str);
		if (len + _size >= _capacity)
			Reserve(2 * (len + _size));
		for (int i = _size; i >= (int)pos; i--)
		{
				_str[i + len] = _str[i];
		}
		for (int i = 0; i < len; i++)
		{
			_str[pos + i] = str[i];
		}
		_size = _size + len;
		return *this;
		
	}

	String& Erase(size_t pos, size_t len)
	{
		for (int i = pos; i < _size; i++)
		{
			if (i + len > _size)
			{
				_str[i - 1] = '\0';
				break;
			}

			_str[i] = _str[i + len];
		}
		_size = strlen(_str);
		return *this;
	}

private:
	char* _str;
	size_t _size;
	size_t _capacity;
};


