#ifndef SCENEMODEL_H_
#define SCENEMODEL_H_

#include <map>
#include <set>
#include <QtCore/QAbstractItemModel>
#include "goat3d.h"

struct SceneNodeData {
	bool visible;
	bool selected;
};

class SceneModel : public QAbstractItemModel {
private:
	Q_OBJECT

	goat3d *scn;
	std::map<goat3d_node*, SceneNodeData> node_data;
	std::set<goat3d_node*> selected;

public:
	SceneModel();
	~SceneModel();

	void set_scene(goat3d *scn);
	void clear_scene();

	SceneNodeData *get_node_data(goat3d_node *node) const;

	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant headerData(int section, Qt::Orientation orient, int role) const;
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	bool hasChildren(const QModelIndex &parent) const;
	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &index) const;

	void selchange(const QModelIndexList &selidx);
};

#endif	// SCENEMODEL_H_
