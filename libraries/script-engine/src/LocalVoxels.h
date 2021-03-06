//
//  LocalVoxels.h
//  hifi
//
//  Created by Clément Brisset on 2/24/14.
//  Copyright (c) 2014 High Fidelity, Inc. All rights reserved.
//

#ifndef __hifi__LocalVoxels__
#define __hifi__LocalVoxels__

#include <QObject>

#include <RegisteredMetaTypes.h>
#include <LocalVoxelsList.h>


/// object allowing JS scripters to use their own local trees
class LocalVoxels : public QObject {
    Q_OBJECT
    
public:
    LocalVoxels(QString name);
    ~LocalVoxels();
    
    /// checks the local voxel tree for a voxel at the specified location and scale
    /// \param x the x-coordinate of the voxel (in meter units)
    /// \param y the y-coordinate of the voxel (in meter units)
    /// \param z the z-coordinate of the voxel (in meter units)
    /// \param scale the scale of the voxel (in meter units)
    Q_INVOKABLE VoxelDetail getVoxelAt(float x, float y, float z, float scale);
    
    /// creates a non destructive voxel in the local tree
    /// \param x the x-coordinate of the voxel (in meter units)
    /// \param y the y-coordinate of the voxel (in meter units)
    /// \param z the z-coordinate of the voxel (in meter units)
    /// \param scale the scale of the voxel (in meter units)
    /// \param red the R value for RGB color of voxel
    /// \param green the G value for RGB color of voxel
    /// \param blue the B value for RGB color of voxel
    Q_INVOKABLE void setVoxelNonDestructive(float x, float y, float z, float scale, uchar red, uchar green, uchar blue);
    
    /// creates a voxel in the local tree
    /// \param x the x-coordinate of the voxel (in meter units)
    /// \param y the y-coordinate of the voxel (in meter units)
    /// \param z the z-coordinate of the voxel (in meter units)
    /// \param scale the scale of the voxel (in meter units)
    /// \param red the R value for RGB color of voxel
    /// \param green the G value for RGB color of voxel
    /// \param blue the B value for RGB color of voxel
    Q_INVOKABLE void setVoxel(float x, float y, float z, float scale, uchar red, uchar green, uchar blue);
    
    /// erase the voxel and its children at the given coordinate
    /// \param x the x-coordinate of the voxel (in meter units)
    /// \param y the y-coordinate of the voxel (in meter units)
    /// \param z the z-coordinate of the voxel (in meter units)
    /// \param scale the scale of the voxel (in meter units)
    Q_INVOKABLE void eraseVoxel(float x, float y, float z, float scale);
    
    /// copy the given subtree onto destination's root node
    /// \param x the x-coordinate of the subtree (in meter units)
    /// \param y the y-coordinate of the subtree (in meter units)
    /// \param z the z-coordinate of the subtree (in meter units)
    /// \param scale the scale of the subtree (in meter units)
    /// \param destination LocalVoxels' destination tree
    Q_INVOKABLE void copyTo(float x, float y, float z, float scale, const QString destination);
    
    ///copy source in the given subtree
    /// \param x the x-coordinate of the subtree (in meter units)
    /// \param y the y-coordinate of the subtree (in meter units)
    /// \param z the z-coordinate of the subtree (in meter units)
    /// \param scale the scale of the subtree (in meter units)
    /// \param source LocalVoxels' source tree
    Q_INVOKABLE void pasteFrom(float x, float y, float z, float scale, const QString source);
    
    /// If the scripting context has visible voxels, this will determine a ray intersection
    Q_INVOKABLE RayToVoxelIntersectionResult findRayIntersection(const PickRay& ray);
    
    /// returns a voxel space axis aligned vector for the face, useful in doing voxel math
    Q_INVOKABLE glm::vec3 getFaceVector(const QString& face);
    
private:
    QString _name;
    StrongVoxelTreePointer _tree;
};




#endif /* defined(__hifi__LocalVoxels__) */
