//-------------------------------------------------------------------
//
//                         Node.h
//
//                   author: Stefan Hachul
//                     date : June 2006
//
//--------------------------------------------------------------------

//Data structure for representing nodes and an int value (needed for class ogdf/list)
//to perform bucket sort.

#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_NODE_H
#define OGDF_NODE_H

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Graph_d.h>
#include <iostream>

namespace ogdf {

class Node
{
  
	friend int value(const Node& A){return A.value;}
	friend ostream &operator<< (ostream & output,const Node & A)
	{   
		output <<"node index "; 
		if(A.vertex == NULL) 
			output<<"nil";
		else
			output<<A.vertex->index();
		output<<" value "<< A.value;
		return output;
	}

  friend istream &operator>> (istream & input,Node & A) {
	  input >> A.value;
	  return input;
  }

  public:
     Node(){vertex = NULL;value = 0;}        //constructor
     ~Node(){;}    //destructor
  

     void set_Node(node v,int a) {vertex = v;value = a;}
     int  get_value() const {return value;}
     node get_node() const {return vertex;}

  private:
     node vertex;
     int value ;

};
}//namespace ogdf
#endif

  
