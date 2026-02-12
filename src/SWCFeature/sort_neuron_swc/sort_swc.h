/*
 *  sort_func.cpp
 *  core functions for sort neuron swc
 *
 *  Created by Wan, Yinan, on 06/20/11.
 *  Changed by  Wan, Yinan, on 06/23/11.
 *  Enable processing of .ano file, add threshold parameter by Yinan Wan, on 01/31/12
 *  Extracted as standalone algorithm module
 */
#ifndef __SORT_SWC_H_
#define __SORT_SWC_H_

#include <QtGlobal>
#include <math.h>
#include "basic_surf_objs.h"
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <QStack>
#include <QQueue>
#include <QDebug>

using namespace std;

#ifndef VOID_VALUE
#define VOID_VALUE 1000000000
#endif
#ifndef MAX_INT
#define MAX_INT 10000000
#endif

#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(NTDIS(a,b)*NTDIS(a,c)))*180.0/3.14159265359)

#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.79768e+308
#endif

QVector< QVector<V3DLONG> > get_neighbors(QList<NeuronSWC> &neurons, const QHash<V3DLONG,V3DLONG> & LUT)
{
    auto idlist=LUT.values();
    std::unique(idlist.begin(),idlist.end());
    std::sort(idlist.begin(),idlist.end());
    int siz = idlist.size();
    QList<int> nlist;
    for(V3DLONG i=0; i<neurons.size(); i++){nlist.append(neurons.at(i).n);}

    QVector< QVector<V3DLONG> > neighbors = QVector< QVector<V3DLONG> >(siz, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neurons.size();i++)
    {
        int pid_old = nlist.lastIndexOf(neurons.at(i).pn);
        if(pid_old<0){
            continue;
        }
        else{
            int pname_old = neurons.at(pid_old).n;
            int cname_old = neurons.at(i).n;
            int pid_new = LUT.value(pname_old);
            int cid_new = LUT.value(cname_old);
            if((pid_new>=siz) || (cid_new>=siz)){
                cerr << QString("Out of range [0, %1]: pid:%2; cid:%3").arg(siz).arg(pid_new).arg(cid_new).toStdString() << endl;
            }
            if(!neighbors.at(cid_new).contains(pid_new)){
                neighbors[cid_new].push_back(pid_new);
            }
            if(!neighbors.at(pid_new).contains(cid_new)){
                neighbors[pid_new].push_back(cid_new);
            }
        }
    }
    return neighbors;
}

QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons, QHash<V3DLONG, NeuronSWC> & LUT_newid_to_node)
{
    QHash<V3DLONG,V3DLONG> LUT;
    V3DLONG cur_id=0;
    for (V3DLONG i=0;i<neurons.size();i++)
    {
        V3DLONG j=0;
        for (j=0;j<i;j++)
        {
            if (neurons.at(i).x==neurons.at(j).x
                    && neurons.at(i).y==neurons.at(j).y
                    && neurons.at(i).z==neurons.at(j).z)
            {
                break;
            }
        }
        if(i==j){
            LUT.insert(neurons.at(i).n, cur_id);
            LUT_newid_to_node.insert(cur_id, neurons.at(j));
            cur_id++;
        }
        else{
            LUT.insert(neurons.at(i).n, LUT.value(neurons.at(j).n));
        }
    }
    return (LUT);
};


double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2)
{
    double xx = s1.x-s2.x;
    double yy = s1.y-s2.y;
    double zz = s1.z-s2.z;
    return sqrt(xx*xx+yy*yy+zz*zz);
};

QList<V3DLONG> DFS(QVector< QVector<V3DLONG> > neighbors, V3DLONG newrootid, V3DLONG siz)
{
    QList<V3DLONG> neworder;

    QStack<int> pstack;
    QList<int> visited;
    for(int i=0;i<siz; i++){visited.append(0);}
    visited[newrootid]=1;
    pstack.push(newrootid);
    neworder.append(newrootid);

    bool is_push;
    int pid;
    while(!pstack.isEmpty()){
        is_push = false;
        pid = pstack.top();
        QVector<V3DLONG>::iterator it;
        QVector<V3DLONG> cur_neighbors = neighbors.at(pid);
        for(it=cur_neighbors.begin(); it!=cur_neighbors.end(); ++it)
        {
            if(visited.at(*it)==0)
            {
                pstack.push(*it);
                is_push=true;
                visited[*it]=1;
                neworder.append(*it);
                break;
            }
        }
        if(!is_push){
            pstack.pop();
        }
    }
    return neworder;
};

bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres)
{
    if(neurons.size()==0){
        cerr << "Empty SWC file." << endl;
        return(false);
    }
    QList<V3DLONG> nlist;
    for(int i=0; i<neurons.size(); i++){
        nlist.append(neurons.at(i).n);
    }

    QHash<V3DLONG, NeuronSWC> LUT_newid_to_node;
    QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons, LUT_newid_to_node);

    auto keys=LUT.keys();
    std::sort(keys.begin(),keys.end());

    auto idlist=LUT.values();
    std::unique(idlist.begin(),idlist.end());
    std::sort(idlist.begin(),idlist.end());
    V3DLONG siz = idlist.size();

    QVector< QVector<V3DLONG> > neighbors = get_neighbors(neurons, LUT);

    V3DLONG root = 0;
    if (newrootid==VOID_VALUE)
    {
        for (V3DLONG i=0;i<neurons.size();i++)
            if (neurons.at(i).pn==-1){
                root = idlist.indexOf(LUT.value(neurons.at(i).n));
                qDebug()<<"value of root"<<root;
                break;
            }
    }
    else{
        root = idlist.indexOf(LUT.value(newrootid));
        if (LUT.keys().indexOf(newrootid)==-1)
        {
            cerr << "The new root id you have chosen does not exist in the SWC file." << endl;
            return(false);
        }
    }

    QList<V3DLONG> neworder;
    QList<V3DLONG> cur_neworder;
    QList<V3DLONG> component_id;
    for(int i=0; i<siz; i++){
        component_id.append(0);
    }
    V3DLONG sorted_size = 0;
    int cur_group = 1;

    cur_neworder= DFS(neighbors, root, siz);
    qDebug()<<QString("cur_neworder_first=%1").arg(cur_neworder.size());
    sorted_size += cur_neworder.size();
    neworder.append(cur_neworder);
    for(int i=0; i<cur_neworder.size(); i++){
        component_id[cur_neworder.at(i)] = cur_group;
    }
    cout<<"Done 1st DFS"<<endl;

    while (sorted_size <siz)
    {
        V3DLONG new_root;
        cur_group++;
        for (V3DLONG iter=0;iter<siz;iter++)
        {
            if (!neworder.contains(iter))
            {
                new_root = iter;
                break;
            }
        }
        cur_neworder = DFS(neighbors, new_root, siz);
        qDebug()<<QString("cur_neworder_second=%1").arg(cur_neworder.size());
        sorted_size += cur_neworder.size();
        neworder.append(cur_neworder);
        for(int i=0; i<cur_neworder.size(); i++){
            component_id[cur_neworder.at(i)] = cur_group;
        }
    }
    qDebug()<<"Number of components before making connections"<<cur_group;

    QList<V3DLONG> output_newroot_list;
    if(thres>=0){
        qDebug()<<"find the point in non-group 1 that is nearest to group 1";
        output_newroot_list.append(root);
        while(cur_group>1)
        {
            qDebug()<<"Remaining components: "<<cur_group;
            double min = VOID_VALUE;
            double dist2 = 0;
            int mingroup = 1;

            V3DLONG m1,m2;
            for (V3DLONG ii=0;ii<siz;ii++)
            {
                if (component_id[ii]==1)
                {
                    for (V3DLONG jj=0;jj<siz;jj++)
                        if (component_id[jj]!=1)
                        {
                            dist2 = computeDist2(LUT_newid_to_node.value(ii),
                                                 LUT_newid_to_node.value(jj));
                            if (dist2<min)
                            {
                                min = dist2;
                                mingroup = component_id[jj];
                                m1 = ii;
                                m2 = jj;
                            }
                        }
                }
            }
            for (V3DLONG i=0;i<siz;i++)
            {
                if (component_id[i]==mingroup)
                {
                    component_id[i] = 1;
                }
            }
            qDebug()<<QString("Min distance: %1").arg(min);
            if (min<=thres)
            {
                qDebug()<<QString("New connection is made between %1 and %2").arg(m1).arg(m2);
                qDebug()<<QString("Original node name: %1 and %2")
                          .arg(LUT_newid_to_node.value(m1).n)
                          .arg(LUT_newid_to_node.value(m2).n);
                if(!neighbors.at(m1).contains(m2)){neighbors[m1].push_back(m2);}
                if(!neighbors.at(m2).contains(m1)){neighbors[m2].push_back(m1);}
            }
            else{
                output_newroot_list.append(m2);
            }
            cur_group--;
        }
        qDebug()<<"Number of components after making connections"<<output_newroot_list.size();
    }
    else{
        int tp_group = 0;
        for(int i=0; i<siz; i++){
            if(component_id.at(i) != tp_group){
                output_newroot_list.append(neworder.at(i));
                tp_group = component_id.at(i);
            }
        }
    }

    // DFS sort of the neuronlist after new connections
    for (int i=0;i<siz;i++)
    {
        component_id[i] = 0;
        neworder[i]= VOID_VALUE;
    }
    component_id.clear();
    neworder.clear();
    sorted_size = 0;
    cur_group = 1;

    V3DLONG offset=0;
    for(V3DLONG i=0; i<output_newroot_list.size(); i++)
    {
        V3DLONG new_root = output_newroot_list.at(i);
        V3DLONG cnt = 0;
        cur_neworder= DFS(neighbors, new_root, siz);
        qDebug()<<QString("cur_neworder=%1").arg(cur_neworder.size());
        sorted_size += cur_neworder.size();
        neworder.append(cur_neworder);
        for(int i=0; i<cur_neworder.size(); i++){
            component_id.append(cur_group);
        }
        NeuronSWC S;
        S = LUT_newid_to_node.value(new_root);
        S.n = offset+1;
        S.pn = -1;
        if(S.x !=0 && S.y!=0 && S.z!=0){
            result.append(S);
        }

        cnt++;
        qDebug()<<QString("New root %1:").arg(i)<<S.x<<S.y<<S.z;

        for (V3DLONG ii=offset+1;ii<(sorted_size);ii++)
        {
            for (V3DLONG jj=offset;jj<ii;jj++)
            {
                V3DLONG cid = neworder[ii];
                V3DLONG pid = neworder[jj];
                if (pid!=VOID_VALUE && cid!=VOID_VALUE && neighbors.at(pid).contains(cid))
                {
                        NeuronSWC S;
                        S = LUT_newid_to_node.value(cid);
                        S.n = ii+1;
                        S.pn = jj+1;
                        if(S.x !=0 && S.y!=0 && S.z!=0){
                            result.append(S);
                        }
                        cnt++;
                        break;
                }
            }
        }
        offset += cnt;
    }

    if ((sorted_size)<siz) {
        cerr << QString("Error!\nsorted_size:%1\nsize:%2").arg(sorted_size).arg(siz).toStdString() << endl;
        return false;
    }

    neighbors.clear();
    return(true);
};

#endif
