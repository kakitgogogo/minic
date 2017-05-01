#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include "token.h"
#include "lexer.h"
#include "parser.h"
using namespace std;

int main(int argc, char* argv[])
{
	if(argc != 2) 
	{
		return 1;
	}

	ofstream fout(string(argv[1])+".dot");

	Lexer lexer(argv[1]);
	Parser parser(lexer);

	Node* root = new Node();
	root->argv[0] = parser.parse();

	queue<Node*> q;
	queue<int> idq;
	q.push(root);
	idq.push(0);
	int id = 1;

	fout << "digraph G {" << endl;
	fout << "\tp0[label=root]" << endl;

	while(!q.empty())
	{
		int size = q.size();
		for(int i = 0; i < size; ++i)
		{
			Node* tmp = q.front();
			q.pop();
			int from = idq.front();
			idq.pop();

			int j = 0;
			while(tmp->argv[j] && j < 4)
			{
				if(tmp->argv[j]->kind == NK_NULL) 
				{
					j++;
					continue;
				}
				q.push(tmp->argv[j]);
				idq.push(id);
				fout << "\tp" << id << "[label=" << tmp->argv[j]->to_string() << "]" << endl;
				fout << "\tp" << from << "->p" << id << endl;
				++j;
				++id;
			}
		}
	}

	fout << "}" << endl;
}