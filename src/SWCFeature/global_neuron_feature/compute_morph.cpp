#include "compute_morph.h"
#include <math.h>
#include <iostream>
#include <QList>
#include <QStack>
using namespace std;

#define VOID 1000000000
#define PI 3.14159265359
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(dist(a,b)*dist(a,c)))*180.0/PI)

static double Width=0, Height=0, Depth=0, Diameter=0, Length=0, Volume=0, Surface=0, Hausdorff=0;
static int N_node=0, N_stem=0, N_bifs=0, N_branch=0, N_tips=0, Max_Order=0;
static double Pd_ratio=0, Contraction=0, Max_Eux=0, Max_Path=0, BifA_local=0, BifA_remote=0, Soma_surface=0, Fragmentation=0;
static int rootidx=0;

static QVector<QVector<V3DLONG> > childs;

void computeFeature(const NeuronTree & nt, double * features)
{
	Width=0; Height=0; Depth=0; Diameter=0; Length=0; Volume=0; Surface=0; Hausdorff=0;
	N_node=0; N_stem=0; N_bifs=0; N_branch=0; N_tips=0; Max_Order=0;
	Pd_ratio=0; Contraction=0; Max_Eux=0; Max_Path=0; BifA_local=0; BifA_remote=0; Soma_surface=0; Fragmentation=0;
	rootidx=0;

	V3DLONG neuronNum = nt.listNeuron.size();
	childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
	for (V3DLONG i=0;i<neuronNum;i++)
	{
		V3DLONG par = nt.listNeuron[i].pn;
		if (par<0) continue;
		childs[nt.hashNeuron.value(par)].push_back(i);
	}


	//find the root
	rootidx = VOID;
	QList<NeuronSWC> list = nt.listNeuron;
	for (int i=0;i<list.size();i++)
    {
        if (list.at(i).pn==-1){
            rootidx = i;
            break;
        }
    }

    if (rootidx==VOID){
		cerr<<"the input neuron tree does not have a root, please check your data"<<endl;
		return;
	}


	N_node = list.size();
	N_stem = childs[rootidx].size();
	Soma_surface = 4*PI*(list.at(rootidx).r)*(list.at(rootidx).r);

	computeLinear(nt);
	computeTree(nt);

	features[0] = N_node;
	features[1] = Soma_surface;
	features[2] = N_stem;
	features[3] = N_bifs;
	features[4] = N_branch;
	features[5] = N_tips;
	features[6] = Width;
	features[7] = Height;
	features[8] = Depth;
	features[9] = Diameter;
	features[10] = Length;
	features[11] = Surface;
	features[12] = Volume;
	features[13] = Max_Eux;
	features[14] = Max_Path;
	features[15] = Max_Order;
	features[16] = Contraction;
	features[17] = Fragmentation;
	features[18] = Pd_ratio;
	features[19] = BifA_local;
	features[20] = BifA_remote;
	features[21] = Hausdorff;
}



QVector<V3DLONG> getRemoteChild(int t)
{
	QVector<V3DLONG> rchildlist;
	rchildlist.clear();
	int tmp;
	for (int i=0;i<childs[t].size();i++)
	{
		tmp = childs[t].at(i);
		while (childs[tmp].size()==1)
			tmp = childs[tmp].at(0);
		rchildlist.append(tmp);
	}
	return rchildlist;
}

void computeLinear(const NeuronTree & nt)
{
	double xmin,ymin,zmin;
	xmin = ymin = zmin = VOID;
	double xmax,ymax,zmax;
	xmax = ymax = zmax = 0;
	QList<NeuronSWC> list = nt.listNeuron;
	NeuronSWC soma = list.at(rootidx);

	for (int i=0;i<list.size();i++)
	{
		NeuronSWC curr = list.at(i);
		xmin = min(xmin,curr.x); ymin = min(ymin,curr.y); zmin = min(zmin,curr.z);
		xmax = max(xmax,curr.x); ymax = max(ymax,curr.y); zmax = max(zmax,curr.z);
		if (childs[i].size()==0)
			N_tips++;
		else if (childs[i].size()>1)
			N_bifs++;
		int parent = getParent(i,nt);
		if (parent==VOID) continue;
		double l = dist(curr,list.at(parent));
		Diameter += 2*curr.r;
		Length += l;
		Surface += 2*PI*curr.r*l;
		Volume += PI*curr.r*curr.r*l;
		double lsoma = dist(curr,soma);
		Max_Eux = max(Max_Eux,lsoma);
	}
	Width = xmax-xmin;
	Height = ymax-ymin;
	Depth = zmax-zmin;
	Diameter /= list.size();
}

void computeTree(const NeuronTree & nt)
{
	QList<NeuronSWC> list = nt.listNeuron;
	NeuronSWC soma = nt.listNeuron.at(rootidx);

	double * pathTotal = new double[list.size()];
	int * depth = new int[list.size()];
	for (int i=0;i<list.size();i++)
	{
		pathTotal[i] = 0;
		depth[i] = 0;
	}

	QStack<int> stack = QStack<int>();
	stack.push(rootidx);
	double pathlength,eudist,max_local_ang,max_remote_ang;
	V3DLONG N_ratio = 0, N_Contraction = 0;

	if (childs[rootidx].size()>1)
	{
		double local_ang,remote_ang;
		max_local_ang = 0;
		max_remote_ang = 0;
		int ch_local1 = childs[rootidx][0];
		int ch_local2 = childs[rootidx][1];
		local_ang = angle(list.at(rootidx),list.at(ch_local1),list.at(ch_local2));

		int ch_remote1 = getRemoteChild(rootidx).at(0);
		int ch_remote2 = getRemoteChild(rootidx).at(1);
		remote_ang = angle(list.at(rootidx),list.at(ch_remote1),list.at(ch_remote2));
		if (local_ang==local_ang)
			max_local_ang = max(max_local_ang,local_ang);
		if (remote_ang==remote_ang)
			max_remote_ang = max(max_remote_ang,remote_ang);

		BifA_local += max_local_ang;
		BifA_remote += max_remote_ang;
	}

	int t,tmp,fragment;
	while (!stack.isEmpty())
	{
		t = stack.pop();
		QVector<V3DLONG> child = childs[t];
		for (int i=0;i<child.size();i++)
		{
			N_branch++;
			tmp = child[i];
			if (list[t].r > 0)
			{
				N_ratio ++;
				Pd_ratio += list.at(tmp).r/list.at(t).r;
			}
			pathlength = dist(list.at(tmp),list.at(t));

			fragment = 0;
			while (childs[tmp].size()==1)
			{
				int ch = childs[tmp].at(0);
				pathlength += dist(list.at(ch),list.at(tmp));
				fragment++;
				tmp = ch;
			}
			eudist = dist(list.at(tmp),list.at(t));
			Fragmentation += fragment;
			if (pathlength>0)
			{
				Contraction += eudist/pathlength;
				N_Contraction++;
			}

			int chsz = childs[tmp].size();
			if (chsz>1)
			{
				stack.push(tmp);

				double local_ang,remote_ang;
				max_local_ang = 0;
				max_remote_ang = 0;
				int ch_local1 = childs[tmp][0];
				int ch_local2 = childs[tmp][1];
				local_ang = angle(list.at(tmp),list.at(ch_local1),list.at(ch_local2));

				int ch_remote1 = getRemoteChild(tmp).at(0);
				int ch_remote2 = getRemoteChild(tmp).at(1);
				remote_ang = angle(list.at(tmp),list.at(ch_remote1),list.at(ch_remote2));
				if (local_ang==local_ang)
					max_local_ang = max(max_local_ang,local_ang);
				if (remote_ang==remote_ang)
					max_remote_ang = max(max_remote_ang,remote_ang);

				BifA_local += max_local_ang;
				BifA_remote += max_remote_ang;
			}
			pathTotal[tmp] = pathTotal[t] + pathlength;
			depth[tmp] = depth[t] + 1;
		}
	}

	Pd_ratio /= N_ratio;
	Fragmentation /= N_branch;
	Contraction /= N_Contraction;

	if (N_bifs==0)
	{
		BifA_local = 0;
		BifA_remote = 0;
	}
	else
	{
		BifA_local /= N_bifs;
		BifA_remote /= N_bifs;
	}

	for (int i=0;i<list.size();i++)
	{
		Max_Path = max(Max_Path,pathTotal[i]);
		Max_Order = max(Max_Order,depth[i]);
	}
	delete[] pathTotal; pathTotal = NULL;
	delete[] depth; depth = NULL;
}

void printFeature(double * features)
{
	for (int i=0;i<FNUM;i++)
	{
		switch (i)
		{
			case 0:  cout<<"Number of Nodes:"; break;
			case 1:  cout<<"Soma Surface:\t"; break;
			case 2:  cout<<"Number of Stems:"; break;
			case 3:  cout<<"Number of Bifurcatons:"; break;
			case 4:  cout<<"Number of Branches:"; break;
			case 5:  cout<<"Number of Tips:\t"; break;
			case 6:  cout<<"Overall Width:\t"; break;
			case 7:  cout<<"Overall Height:\t"; break;
			case 8:  cout<<"Overall Depth:\t"; break;
			case 9:  cout<<"Average Diameter:"; break;
			case 10: cout<<"Total Length:\t"; break;
			case 11: cout<<"Total Surface:\t"; break;
			case 12: cout<<"Total Volume:\t"; break;
			case 13: cout<<"Max Euclidean Distance:"; break;
			case 14: cout<<"Max Path Distance:\t\t"; break;
			case 15: cout<<"Max Branch Order:\t\t"; break;
			case 16: cout<<"Average Contraction:\t\t"; break;
			case 17: cout<<"Average Fragmentation:\t\t"; break;
			case 18: cout<<"Average Parent-daughter Ratio:\t"; break;
			case 19: cout<<"Average Bifurcation Angle Local:"; break;
			case 20: cout<<"Average Bifurcation Angle Remote:"; break;
			case 21: cout<<"Hausdorff Dimension:\t\t"; break;
		}
		cout<<"\t"<<features[i]<<endl;
	}
}
