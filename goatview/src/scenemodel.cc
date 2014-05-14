#include <assert.h>
#include "scenemodel.h"

SceneModel::SceneModel()
{
	scn = 0;
}

SceneModel::~SceneModel()
{
	clear_scene();
}

void SceneModel::set_scene(goat3d *scn)
{
	clear_scene();
	this->scn = scn;

	if(!scn) return;

	// create the SceneNodeData for each node
	int num_nodes = goat3d_get_node_count(scn);
	for(int i=0; i<num_nodes; i++) {
		goat3d_node *node = goat3d_get_node(scn, i);

		SceneNodeData data;
		data.visible = true;

		node_data[node] = data;
	}
}

void SceneModel::clear_scene()
{
	node_data.clear();
	scn = 0;
}

SceneNodeData *SceneModel::get_node_data(goat3d_node *node) const
{
	auto it = node_data.find(node);
	if(it == node_data.end()) {
		return 0;
	}
	return (SceneNodeData*)&it->second;
}

Qt::ItemFlags SceneModel::flags(const QModelIndex &index) const
{
	if(!index.isValid()) {
		return 0;
	}

	Qt::ItemFlags res = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	if(index.column() == 1) {
		res |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
	}
	return res;
}

QVariant SceneModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid()) {
		return QVariant();
	}

	goat3d_node *node = (goat3d_node*)index.internalPointer();
	SceneNodeData *data = get_node_data(node);

	switch(index.column()) {
	case 0:
		if(role == Qt::DisplayRole) {
			return QString(goat3d_get_node_name(node));
		}
		break;

	case 1:
		if(role == Qt::CheckStateRole && data) {
			return data->visible ? Qt::Checked : Qt::Unchecked;
		}
		break;

	default:
		break;
	}

	return QVariant();
}

bool SceneModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!index.isValid()) {
		return false;
	}

	goat3d_node *node = (goat3d_node*)index.internalPointer();
	SceneNodeData *data = get_node_data(node);
	assert(data);

	switch(index.column()) {
	case 1:
		if(role == Qt::CheckStateRole) {
			data->visible = value.toBool();
			emit dataChanged(index, index);
			return true;
		}
		break;

	default:
		break;
	}
	return false;
}

QVariant SceneModel::headerData(int section, Qt::Orientation orient, int role) const
{
	if(orient == Qt::Horizontal && role == Qt::DisplayRole) {
		switch(section) {
		case 0:
			return QString("name");
		case 1:
			return QString("vis");
		default:
			return QString("???");
		}
	}
	return QVariant();
}

int SceneModel::rowCount(const QModelIndex &parent) const
{
	if(!scn) return 0;

	if(!parent.isValid()) {
		// return the number of root nodes
		int num_nodes = goat3d_get_node_count(scn);
		int num_root_nodes = 0;
		for(int i=0; i<num_nodes; i++) {
			goat3d_node *node = goat3d_get_node(scn, i);
			if(!goat3d_get_node_parent(node)) {
				++num_root_nodes;
			}
		}
		return num_root_nodes;
	}

	goat3d_node *pnode = (goat3d_node*)parent.internalPointer();
	return goat3d_get_node_child_count(pnode);
}

int SceneModel::columnCount(const QModelIndex &parent) const
{
	return 2;
}

bool SceneModel::hasChildren(const QModelIndex &parent) const
{
	if(!parent.isValid()) {
		return true;
	}

	goat3d_node *pnode = (goat3d_node*)parent.internalPointer();
	return goat3d_get_node_child_count(pnode) > 0;
}

QModelIndex SceneModel::index(int row, int column, const QModelIndex &parent) const
{
	if(!scn) {
		return QModelIndex();
	}

	goat3d_node *node = 0;

	if(!parent.isValid()) {
		int num_nodes = goat3d_get_node_count(scn);
		int idx = 0;
		for(int i=0; i<num_nodes; i++) {
			goat3d_node *n = goat3d_get_node(scn, i);
			if(!goat3d_get_node_parent(n)) {
				if(idx == row) {
					node = n;
					break;
				}
				idx++;
			}
		}

		if(idx != row) {
			return QModelIndex();	// failed
		}
	} else {
		goat3d_node *pnode = (goat3d_node*)parent.internalPointer();
		node = goat3d_get_node_child(pnode, row);
	}

	if(!node) {
		return QModelIndex();
	}
	return createIndex(row, column, (void*)node);
}

QModelIndex SceneModel::parent(const QModelIndex &index) const
{
	if(!index.isValid()) {
		return QModelIndex();	// root node
	}

	goat3d_node *node = (goat3d_node*)index.internalPointer();
	goat3d_node *parent = node ? goat3d_get_node_parent(node) : 0;

	if(!parent) {
		return QModelIndex();
	}

	// find out which child of its parent is our parent
	int pidx = -1;

	goat3d_node *grandparent = goat3d_get_node_parent(parent);
	if(grandparent) {
		int num_children = goat3d_get_node_child_count(grandparent);
		for(int i=0; i<num_children; i++) {
			if(goat3d_get_node_child(grandparent, i) == parent) {
				pidx = i;
				break;
			}
		}
	} else {
		int idx = 0;
		int num_nodes = goat3d_get_node_count(scn);
		for(int i=0; i<num_nodes; i++) {
			goat3d_node *n = goat3d_get_node(scn, i);
			if(!goat3d_get_node_parent(n)) {
				if(n == parent) {
					pidx = idx;
					break;
				}
				idx++;
			}
		}
	}

	if(pidx == -1) {
		fprintf(stderr, "%s: wtf?\n", __FUNCTION__);
		return QModelIndex();	// failed
	}

	return createIndex(pidx, 0, (void*)parent);
}
