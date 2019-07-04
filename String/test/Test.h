#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
using namespace std;

class Solution {
public:
	//给定只含 "I"（增大）或 "D"（减小）的字符串 S ，令 N = S.length。返回[0, 1, ..., N] 的任意排列 A 使得对于所有 i = 0, ..., N - 1，都有：1.如果 S[i] == "I"，那么 A[i] < A[i + 1]，2.如果 S[i] == "D"，那么 A[i] > A[i + 1]
	vector<int> diStringMatch(string S) {
		vector<int> n;
		int max = S.size();
		int min = 0;

		for (int i = 0; i < S.size(); i++){
			if (i == S.size() - 1){
				if (S[i] == 'I'){
					n.push_back(min);
					n.push_back(max);
				}
				else{
					n.push_back(max);
					n.push_back(min);
				}
			}
			else{
				if (S[i] == 'I'){
					n.push_back(min);
					min++;
				}
				else{
					n.push_back(max);
					max--;
				}
			}
		}
		return n;
	}


	//给定一个字符串 s 和一个非空字符串 p，找到 s 中所有是 p 的字母异位词的子串，返回这些子串的起始索引。
	//暴力解法，有缺陷
	vector<int> findAnagrams_1(string s, string p) {
		int len = p.size();
		vector<int> v;
		vector<int> arr;
		if (s.size() == 0 || s.size() < p.size())
			return v;
		for (int i = 0; i < len; i++)
		{
			arr.push_back(0);
		}

		for (int i = 0; i <= s.size() - len; i++)
		{
			for (int j = i; j < i + len; j++)
			{
				for (int k = 0; k < len; k++)
				{
					if (s[j] == p[k] && arr[k] == 0)
					{
						arr[k] = 1;
						break;
					}
				}
			}

			int num = 0;
			for (int j = 0; j < len; j++)
			{
				num += arr[j];
			}
			if (num == len)
				v.push_back(i);

			for (int i = 0; i < len; i++)
			{
				arr[i] = 0;
			}
		}

		return v;
	}

	//滑动窗口思想
	vector<int> findAnagrams_2(string s, string p) {
		vector<int> v;
		int len = p.size();
		int left = 0, right = 0;
		int num = 0;
		unordered_map<char, int> window;
		unordered_map<char, int> needs;
		for (int i = 0; i < len; i++)
			needs[p[i]]++;

		while (right < s.size())
		{
			char c1 = s[right];
			if (needs.count(c1))
			{
				window[c1]++;
				if (window[c1] == needs[c1])
					num++;
			}
			right++;

			while (num == needs.size())
			{
				if (right - left == len)
				{
					v.push_back(left);
				}

				char c2 = s[left];
				if (needs.count(c2))
				{
					window[c2]--;
					if (window[c2] < needs[c2])
						num--;
				}
				left++;
			}
		}
		return v;
	}



};