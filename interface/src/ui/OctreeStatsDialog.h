//
//  OctreeStatsDialog.h
//  interface
//
//  Created by Brad Hefta-Gaub on 7/19/13.
//  Copyright (c) 2013 High Fidelity, Inc. All rights reserved.
//

#ifndef __hifi__OctreeStatsDialog__
#define __hifi__OctreeStatsDialog__

#include <QDialog>
#include <QFormLayout>
#include <QLabel>

#include <OctreeSceneStats.h>

#define MAX_STATS 100
#define MAX_VOXEL_SERVERS 50
#define DEFAULT_COLOR 0

class OctreeStatsDialog : public QDialog {
    Q_OBJECT
public:
    // Sets up the UI
    OctreeStatsDialog(QWidget* parent, NodeToOctreeSceneStats* model);
    ~OctreeStatsDialog();

signals:
    void closed();

public slots:
    void reject();
    void moreless(const QString& link);

protected:
    // State <- data model held by BandwidthMeter
    void paintEvent(QPaintEvent*);

    // Emits a 'closed' signal when this dialog is closed.
    void closeEvent(QCloseEvent*);

    int AddStatItem(const char* caption, unsigned colorRGBA = DEFAULT_COLOR);
    void RemoveStatItem(int item);
    void showAllOctreeServers();

    void showOctreeServersOfType(int& serverNumber, NodeType_t serverType, 
                    const char* serverTypeName, NodeToJurisdictionMap& serverJurisdictions);

private:

    typedef enum { LESS, MORE, MOST } details;

    QFormLayout* _form;
    QLabel* _labels[MAX_STATS];
    NodeToOctreeSceneStats* _model;
    int _statCount;
    
    int _sendingMode;
    int _serverVoxels;
    int _localVoxels;
    int _localVoxelsMemory;
    int _voxelsRendered;
    int _voxelServerLables[MAX_VOXEL_SERVERS];
    int _voxelServerLabelsCount;
    details _extraServerDetails[MAX_VOXEL_SERVERS];
};

#endif /* defined(__interface__OctreeStatsDialog__) */

