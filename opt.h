#ifndef OPT_H
#define OPT_H
#include <string>
#include <vector>
#include "type.h"
using namespace std;
//class DagNode {
//	string left;
//	string right;
//	string name;
//	char op;
//public:
//
//};


class Block {
	vector<int> prev;
	vector<int> post;
	vector<string> def;
	vector<string> use;
	vector<string> in;
	vector<string> out;
	vector<FourYuanItem> codes;
	vector<string> tmps;
public:
	Block(vector<FourYuanItem> codes1) {
		codes.swap(codes1);
	}
	void addToTmps(string tmp) {
		tmps.push_back(tmp);
	}
	void addToPrev(int i) {
		prev.push_back(i);
	}
	void addToPost(int i) {
		post.push_back(i);
	}
	void addToUse(string var) {
		use.push_back(var);
	}
	void addToDef(string var) {
		def.push_back(var);
	}
	void addToIn(string var) {
		in.push_back(var);
	}
	void addToOut(string var) {
		out.push_back(var);
	}
	void setCodes(vector<FourYuanItem> codes1) {
		codes.swap(codes1);
	}
	bool hasAdded(string var) {
		for (int i = 0; i < use.size(); i++) {
			if (use[i] == var) {
				return true;
			}
		}
		for (int i = 0; i < def.size(); i++) {
			if (def[i] == var) {
				return true;
			}
		}
		return false;
	}
	vector<string> getTmps() {
		return tmps;
	}
	vector<int> getPrev() {
		return prev;
	}
	vector<int> getPost() {
		return post;
	}
	vector<string> getIn() {
		return in;
	}
	vector<string> getOut() {
		return out;
	}
	vector<FourYuanItem> getCodes() {
		return codes;
	}
	bool calIn() {
		bool flag = false;
		for (int i = 0; i < use.size(); i++) {
			if (inIn(use[i]) == false) {
				flag = true;
				addToIn(use[i]);
			}
		}
		for (int i = 0; i < out.size(); i++) {
			if (inDef(out[i]) == false && inIn(out[i]) == false) {
				flag = true;
				addToIn(out[i]);
			}
		}
		return flag;
	}

	bool inDef(string var) {
		for (int i = 0; i < def.size(); i++) {
			if (var == def[i]) {
				return true;
			}
		}
		return false;
	}

	bool inIn(string var) {
		for (int i = 0; i < in.size(); i++) {
			if (var == in[i]) {
				return true;
			}
		}
		return false;
	}

	bool inOut(string var) {
		for (int i = 0; i < out.size(); i++) {
			if (var == out[i]) {
				return true;
			}
		}
		return false;
	}
};
#endif