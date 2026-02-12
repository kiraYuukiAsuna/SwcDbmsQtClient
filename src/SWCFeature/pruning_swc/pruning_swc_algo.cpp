#include "pruning_swc_algo.h"
#include <math.h>
#include <iostream>
#include <QDebug>

using namespace std;

#define VOID 1000000000
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

// Minimal struct for distance calculation (replaces MyMarker dependency)
struct SimplePoint {
    double x, y, z;
};

#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

NeuronTree prune_swc_simple(NeuronTree nt, double length, bool& pruned){
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
    double *segment_length = new double[neuronNum];
    V3DLONG *parent_id = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        segment_length[i] = 100000.00;
        parent_id[i] = -1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QList<NeuronSWC> list = nt.listNeuron;
    for (int i=0;i<list.size();i++)
    {
        if (childs[i].size()==0  && list.at(i).parent >=0)
        {
            int parent_tip = getParent(i,nt);
            SimplePoint curr_node, parent_node;
            curr_node.x = list.at(i).x;
            curr_node.y = list.at(i).y;
            curr_node.z = list.at(i).z;

            parent_node.x = list.at(parent_tip).x;
            parent_node.y = list.at(parent_tip).y;
            parent_node.z = list.at(parent_tip).z;
            double index_tip = dist(curr_node,parent_node);

            while(childs[parent_tip].size()<2)
            {
                SimplePoint curr_node, parent_node;

                curr_node.x = list.at(parent_tip).x;
                curr_node.y = list.at(parent_tip).y;
                curr_node.z = list.at(parent_tip).z;

                int newParent = getParent(parent_tip,nt);
                if(newParent == 1000000000) break;

                parent_node.x = list.at(newParent).x;
                parent_node.y = list.at(newParent).y;
                parent_node.z = list.at(newParent).z;

                index_tip += dist(curr_node,parent_node);

                parent_tip = getParent(parent_tip,nt);

                if(parent_tip == 1000000000)    break;
             }

            int parent_index = parent_tip;

            if(index_tip < length)
            {
                flag[i] = -1;
                segment_length[i] = index_tip;
                parent_id[i] = parent_index;
                int parent_tip = getParent(i,nt);
                while(childs[parent_tip].size()<2)
                {
                    flag[parent_tip] = -1;
                    segment_length[parent_tip] = index_tip;
                    parent_id[parent_tip] = parent_index;
                    parent_tip = getParent(parent_tip,nt);
                    if(parent_tip == 1000000000)
                        break;
                }
                if(parent_tip != 1000000000 && segment_length[parent_tip] > index_tip)
                    segment_length[parent_tip]  = index_tip;
            }
        }
    }

    //NeuronTree structure
    NeuronTree nt_prunned;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    //set node
    NeuronSWC S;
    for (int i=0;i<list.size();i++)
    {
        if(flag[i] == 1 || (flag[i] != 1 && (segment_length[i] > segment_length[parent_id[i]])))
        {
             NeuronSWC curr = list.at(i);
             S.n 	= curr.n;
             S.type 	= curr.type;
             S.x 	= curr.x;
             S.y 	= curr.y;
             S.z 	= curr.z;
             S.r 	= curr.r;
             S.pn 	= curr.pn;
             S.seg_id = curr.seg_id;
             S.level = curr.level;
             S.creatmode = curr.creatmode;
             S.timestamp = curr.timestamp;
             S.tfresindex = curr.tfresindex;

             listNeuron.append(S);
             hashNeuron.insert(S.n, listNeuron.size()-1);
        }
   }

    if(flag) {delete[] flag; flag = 0;}
    if(segment_length) {delete[] segment_length; segment_length = 0;}
    if(parent_id) {delete[] parent_id; parent_id = 0;}

    if(listNeuron.size()==0){
        qDebug()<<"Reached the last branch, no pruning will be performed.";
        nt_prunned.deepCopy(nt);
        pruned = 0;
    }
    else{
        if(listNeuron.size()<list.size()){
            pruned = 1;
        }
        else{
            pruned = 0;
        }
       nt_prunned.n = -1;
       nt_prunned.on = true;
       nt_prunned.listNeuron = listNeuron;
       nt_prunned.hashNeuron = hashNeuron;
    }
   return nt_prunned;
}

NeuronTree prune_swc_iterative(NeuronTree nt, double length, bool& prunned){
    int rounds = 0;
    prunned = 1;
    NeuronTree nt_prunned;
    nt_prunned.deepCopy(nt);
    while(prunned){
        nt_prunned = prune_swc_simple(nt_prunned, length, prunned);
        rounds++;
        qDebug()<<"Iteration "<<rounds<<": current size="<<nt_prunned.listNeuron.size();
    }

    rounds --;
    qDebug()<<(qPrintable(QString::number(rounds)+" iterations of prunning performed"));
    if(rounds>0){
        qDebug()<<("Some branches of SWC have been pruned.");
        prunned = 1;
    }
    else{
        qDebug()<<("No branch has been pruned.");
        prunned = 0;
    }
    return nt_prunned;
}
