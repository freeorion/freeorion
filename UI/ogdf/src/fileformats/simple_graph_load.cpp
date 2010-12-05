/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of simple graph loaders.
 * 
 * See header-file simple_graph_load.h for more information.
 * 
 * \author Markus Chimani, Carsten Gutwenger
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 *
 * Copyright (C). All rights reserved.
 * See README.txt in the root directory of the OGDF installation for details.
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/
 
#include <ogdf/fileformats/simple_graph_load.h>
#include <ogdf/basic/Logger.h>
#include <ogdf/basic/HashArray.h>
#include <ogdf/basic/String.h>
#include <string.h>

#define SIMPLE_LOAD_BUFFER_SIZE 2048

namespace ogdf {

bool loadRomeGraph(Graph &G, const char *fileName) {
	std::ifstream is(fileName);
	if(!is.good()) return false;
	return loadRomeGraphStream(G, is);
}

bool loadRomeGraphStream(Graph &G, std::istream& fileStream) {
	G.clear();

	char buffer[SIMPLE_LOAD_BUFFER_SIZE];
	bool readNodes = true;
	Array<node> indexToNode(1,250,0);

	while(!fileStream.eof())
	{
		fileStream.getline(buffer, SIMPLE_LOAD_BUFFER_SIZE-1);

		if(readNodes) {
			if(buffer[0] == '#') {
				readNodes = false;
				continue;
			}

			int index;
			sscanf(buffer, "%d", &index);
			if(index < 1 || index > 250 || indexToNode[index] != 0) {
				Logger::slout() << "loadRomeGraph: illegal node index!\n";
				return false;
			}

			indexToNode[index] = G.newNode();

		} else {
			int index, dummy, srcIndex, tgtIndex;
			sscanf(buffer, "%d%d%d%d", &index, &dummy, &srcIndex, &tgtIndex);

			if(buffer[0] == 0)
				continue;

			if(srcIndex < 1 || srcIndex > 250 || tgtIndex < 1 || tgtIndex > 250 ||
				indexToNode[srcIndex] == 0 || indexToNode[tgtIndex] == 0)
			{
				Logger::slout() << "loadRomeGraph: illegal node index in edge specification.\n";
				return false;
			}

			G.newEdge(indexToNode[srcIndex], indexToNode[tgtIndex]);
		}
	}
	return true;
}

bool loadSimpleGraph(Graph &G, const char *fileName) {
	std::ifstream is(fileName);
	if(!is.good()) return false;
	return loadSimpleGraphStream(G, is);
}
bool loadSimpleGraphStream(Graph &G, std::istream& fileStream) {
	G.clear();

	char buffer[SIMPLE_LOAD_BUFFER_SIZE];
	int numN = 0, numE = 0;
	
	//try to read the two first lines to get the graph size
	if (!fileStream.eof())
	{
		char* pch;
		//contains the name 
		fileStream.getline(buffer, SIMPLE_LOAD_BUFFER_SIZE-1);
		pch = strtok(buffer, " ");
		if (strcmp(pch, "*BEGIN") != 0) return false;
		if (!fileStream.eof())
		{	//contains the size of the graph
			fileStream.getline(buffer, SIMPLE_LOAD_BUFFER_SIZE-1);
			pch = strtok(buffer, " ");
		    if (strcmp(pch, "*GRAPH") != 0) return false;
		    //now read the number of nodes
		    pch = strtok(NULL, " ");
		    if (pch == NULL) return false;
		    numN = atoi(pch);
		    //now read the number of edges
		    pch = strtok(NULL, " ");
		    if (pch == NULL) return false;
		    numE = atoi(pch);
		}
		else return false;	
	}
	else return false;
	
	if (numN == 0) return true;
	
	Array<node> indexToNode(1,numN,0);
	for (int i = 1; i <= numN; i++)
	{
		//we assign new indexes here if they are not consecutive 
		//starting from 1 in the file!
		indexToNode[i] = G.newNode();
	}

	while(!fileStream.eof())
	{
		fileStream.getline(buffer, SIMPLE_LOAD_BUFFER_SIZE-1);
		
		if(buffer[0] == 0)
			continue;
		    
		int srcIndex, tgtIndex;
		sscanf(buffer, "%d%d", &srcIndex, &tgtIndex);
		char* pch;
		pch = strtok(buffer, " ");
		if ( (strcmp(pch, "*END") == 0) || (strcmp(pch, "*CHECKSUM") == 0) ) 
		    continue;
		
		if(srcIndex < 1 || srcIndex > numN || tgtIndex < 1 || tgtIndex > numN)
			{
				Logger::slout() << "loadSimpleGraphStream: illegal node index in edge specification.\n";
				return false;
			}

			G.newEdge(indexToNode[srcIndex], indexToNode[tgtIndex]);
	}
	return true;
}
#define YG_NEXTBYTE(x) x = fgetc(lineStream);	if(x == EOF || x == '\n') { Logger::slout() << "loadYGraph: line too short!"; return false; } x &= 0x3F;

bool loadYGraph(Graph &G, FILE *lineStream) {
	G.clear();

	char c,s;
	int n,i,j;
	
	YG_NEXTBYTE(n);
	Array<node> A(n);
	for(i=n; i-->0;)
		A[i] = G.newNode();
	
	s = 0;
	for(i = 1; i<n; ++i) {
		for(j = 0; j<i; ++j) {
			if(!s) {
				YG_NEXTBYTE(c);
				s = 5;
			} else --s;
			if(c & (1 << s))
				G.newEdge(A[i],A[j]);
		}
	}

	c = fgetc(lineStream);
	if(c != EOF && c != '\n') { 
		Logger::slout(Logger::LL_MINOR) << "loadYGraph: Warning: line too long! ignoring...";
	}
	return true;
}

bool loadBenchHypergraph(Graph &G, List<node>& hypernodes, List<edge>* shell, const char *fileName) {
	std::ifstream is(fileName);
	if(!is.good()) return false;
	return loadBenchHypergraphStream(G, hypernodes, shell, is);
}

bool loadPlaHypergraph(Graph &G, List<node>& hypernodes, List<edge>* shell, const char *fileName) {
	std::ifstream is(fileName);
	if(!is.good()) return false;
	return loadPlaHypergraphStream(G, hypernodes, shell, is);
}

int extractIdentifierLength(char* from, int line) {
	int p = 1;
	while(from[p]!=',' && from[p]!=')' && from[p]!=' ' && from[p]!='(') { 
		++p;
		if(from[p]=='\0') {
			cerr << "Loading Hypergraph: Error in line " << line << 
				". Expected comma, bracket or whitespace before EOL; Ignoring.\n";
			break;
		}
	}
	return p;
}

int newStartPos(char* from, int line) {
	int p = 0;
	while(from[p]=='\t' || from[p]==' ' || from[p]==',') { 
		++p;
		if(from[p]=='\0') {
			cerr << "Loading Hypergraph: Error in line " << line << 
				". Expected whitespace or delimiter before EOL; Ignoring.\n";
			break;
		}
	}
	
	return p;
}

int findOpen(char* from, int line) {
	int p = 0;
	while(from[p]!='(') { 
		++p;
		if(from[p]=='\0') {
			cerr << "Loading Hypergraph: Error in line " << line << 
				". Expected opening bracket before EOL; Ignoring.\n";
			break;
		}
	}
	return p;
}


String inName(const String& s) {
	size_t n = s.length();
	char *t = new char[n+4];
	ogdf::strcpy(t,s.length()+1,s.cstr());
	t[n] = '%';t[n+1] = '$';t[n+2] = '@';t[n+3] = '\0';
	String u(t);
	delete[] t;
	return u;
}

bool loadBenchHypergraphStream(Graph &G, List<node>& hypernodes, List<edge>* shell, std::istream& fileStream) {
	G.clear();
	hypernodes.clear();
	if(shell) shell->clear();
	node si,so;
	
	char buffer[SIMPLE_LOAD_BUFFER_SIZE];
	
//	Array<node> indexToNode(1,250,0);

	HashArray<String,node> hm(0);
	
	if(shell) {
//		hypernodes.pushBack( si=G.newNode() );
//		hypernodes.pushBack( so=G.newNode() );
//		shell.pushBack( G.newEdge( si, so ) );		
		shell->pushBack( G.newEdge( si=G.newNode(), so=G.newNode() ) );
	}

	int line = 0;
	while(!fileStream.eof())
	{	
		++line;
		fileStream.getline(buffer, SIMPLE_LOAD_BUFFER_SIZE-1);
		size_t l = strlen(buffer);
		if( l > 0 && buffer[l-1]=='\r' ) { // DOS line
			buffer[l-1]='\0';
		}
		if(!strlen(buffer) || buffer[0]==' ' || buffer[0]=='#') continue;
		if(!strncmp("INPUT(",buffer,6)) {
			String s(extractIdentifierLength(buffer+6, line),buffer+6);
			node n = G.newNode();
			hm[s] = n;
			hypernodes.pushBack(n);
			if(shell) shell->pushBack( G.newEdge(si,n) );
//			cout << "input: " << s << " -> " << n->index() << "\n";					
		} else if(!strncmp("OUTPUT(",buffer,7)) {
			String s(extractIdentifierLength(buffer+7, line),buffer+7);
			node n = G.newNode();
			hm[s] = n;
			hypernodes.pushBack(n);
			if(shell) shell->pushBack( G.newEdge(n,so) );								
//			cout << "output: " << s << " -> " << n->index() << "\n";					
		} else {
			int p = extractIdentifierLength(buffer, line);
			String s(p,buffer); // gatename
			node m = hm[s]; // found as outputname -> refOut
			if(!m) {
				m = hm[inName(s)]; // found as innernode input.
				if(!m) { // generate it anew.
					node in = G.newNode();
					node out = G.newNode();
					hm[inName(s)] = in;
					hm[s] = out;
					hypernodes.pushBack(out);
					G.newEdge(in,out);
					m = in;
				}	
			}
			p = findOpen(buffer, line);
			do {
				++p;
				p += newStartPos(buffer+p, line);
				int pp = extractIdentifierLength(buffer+p, line);
				String s(pp,buffer+p);
				p += pp;
				node mm = hm[s];
				if(!mm) {
					// new
					node in = G.newNode();
					node out = G.newNode();
					hm[inName(s)] = in;
					hm[s] = out;
					hypernodes.pushBack(out);
					G.newEdge(in,out);
					mm = out;
				}
				G.newEdge(mm,m);
//				cout << "Edge: " << s << "(" << hm[s]->index() << ") TO " << m->index() << "\n";
			} while(buffer[p] == ',');			
		}		
	}	
	
	return true;
}

bool loadPlaHypergraphStream(Graph &G, List<node>& hypernodes, List<edge>* shell, std::istream& fileStream) {
	G.clear();
	hypernodes.clear();
	if(shell) shell->clear();
	node si,so;
	
	int i;
	int numGates;
	fileStream >> numGates;
//	cout << "numGates=" << numGates << "\n";
	
	Array<node> outport(1,numGates);
	for(i = 1; i<=numGates; ++i) {
		node out = G.newNode();
		outport[i] = out;
		hypernodes.pushBack(out);
	}
	
	for(i = 1; i<=numGates; ++i) {
		int id, type, numinput;
		fileStream >> id >> type >> numinput;
//		cout << "Gate=" << i << ", type=" << type << ", numinput=" << numinput << ":";
		if(id != i) cerr << "Error loading PLA hypergraph: ID and linenum does not match\n";
		node in = G.newNode();
		G.newEdge(in,outport[i]);
		for(int j=0; j<numinput; ++j) {
			int from;
			fileStream >> from;
//			cout << " " << from;
			G.newEdge(outport[from],in);
		}
//		cout << "\n";
		fileStream.ignore(500,'\n');
	}
	
	if(shell) {
		shell->pushBack( G.newEdge( si=G.newNode(), so=G.newNode() ) );
		node n;
		forall_nodes(n,G) {
			if(n->degree()==1) {
				if(n->firstAdj()->theEdge()->source()==n) { //input
					shell->pushBack( G.newEdge( si, n ) );
				} else { // output
					shell->pushBack( G.newEdge( n, so ) );
				}
			}
		}
	}
	
	return true;
}

}
