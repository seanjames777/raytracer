/**
 * @file kdbuilder.cpp
 *
 * @author Sean James
 */

#include <kdtree/kdbuilder.h>
#include <timer.h>
#include <iostream> // TODO

// TODO: the enqueue() function can fail

KDBuilder::KDBuilder() {
}

KDBuilder::~KDBuilder() {
}

void *KDBuilder::prepareWorkerThread(int idx) {
	return nullptr;
}

void KDBuilder::destroyWorkerThread(void *threadCtx) {
}

void KDBuilder::partition(void *threadCtx, float dist, int dir, const std::vector<Triangle *> & triangles,
        std::vector<Triangle *> & left, std::vector<Triangle *> & right,
        enum PlanarMode & planarMode)
{
    // TODO: By keeping references to triangles in the SAH event list, we could
    // avoid this whole loop.

    for (auto it = triangles.begin(); it != triangles.end(); it++) {
        Triangle *tri = *it;

		float min = tri->bbox._min.v[dir];
		float max = tri->bbox._max.v[dir];

        if (min == dist && max == dist) {
            // TODO bitwise optimization?
            if (planarMode == PLANAR_LEFT)
                left.push_back(tri);
            else if (planarMode == PLANAR_RIGHT)
                right.push_back(tri);
            else {
                left.push_back(tri);
                right.push_back(tri);
            }
        }
        else {
            if (min < dist)
                left.push_back(tri);

            if (max > dist)
                right.push_back(tri);
        }
    }
}

void KDBuilder::buildLeafNode(KDBuilderQueueNode *q_node) {
    // Set up triangles, and pack them together for locality
    unsigned int num_triangles = (unsigned int)q_node->triangles.size();

    //SetupTriangle *setup = (SetupTriangle *)malloc(sizeof(SetupTriangle) * num_triangles);
    // TODO multiple constructor calls, etc.
    SetupTriangle *setup = new SetupTriangle[num_triangles];

    for (unsigned int i = 0; i < num_triangles; i++)
        setup[i] = SetupTriangle(*q_node->triangles[i]); // TODO constructor in place

    // TODO: Might want to change constructor of KDNode to do this stuff,
    // especially to not mix allocations

    // TODO: handle overflow of size?

	KDNode *node = q_node->node;

	node->left = NULL;
	node->right = NULL;
	node->split_dist = 0.0f;
	node->triangles = setup;
	node->flags = num_triangles | KD_IS_LEAF;
}

void KDBuilder::buildInnerNode(void *threadCtx, KDBuilderQueueNode *q_node, int dir, float split,
	enum PlanarMode planarMode, int depth)
{
	KDBuilderQueueNode *left = new KDBuilderQueueNode();
	KDBuilderQueueNode *right = new KDBuilderQueueNode();

	left->node = new KDNode();
	left->depth = depth + 1;
	left->refCount = 2;
	left->parent = q_node;

	right->node = new KDNode();
	right->depth = depth + 1;
	right->refCount = 2;
	right->parent = q_node;

	q_node->bounds.split(split, dir, left->bounds, right->bounds);

	// TODO might want to reserve space to avoid allocations
	partition(threadCtx, split, dir, q_node->triangles, left->triangles, right->triangles, planarMode);

	KDNode *node = q_node->node;

	node->left = left->node;
	node->right = right->node;
	node->triangles = NULL;
	node->split_dist = split;
	node->flags = (KDNodeFlags)dir;

    queue_lock.lock();
    node_queue.enqueue(left);
    node_queue.enqueue(right);
    outstanding_nodes += 2;
	queue_lock.unlock();
}

void KDBuilder::buildNode(void *threadCtx, KDBuilderQueueNode *q_node) {
    // TODO: possibly traverse after construction to remove useless cells, etc.

    int dir;
    float split;
    enum PlanarMode planarMode;

	if (splitNode(threadCtx, q_node->bounds, q_node->triangles, q_node->depth, dir, split, planarMode))
		buildInnerNode(threadCtx, q_node, dir, split, planarMode, q_node->depth);
	else
		buildLeafNode(q_node);
}

AABB KDBuilder::buildAABB(const std::vector<Triangle *> & triangles) {
    if (triangles.size() == 0)
        return AABB();

    AABB box = triangles[0]->bbox;

    for (auto it = triangles.begin(); it != triangles.end(); it++)
        box.join((*it)->bbox);

    return box;
}

void KDBuilder::worker_thread() {
	void *ctx = prepareWorkerThread(0); // TODO: actual index

    util::queue<KDBuilderQueueNode *> local_queue;

	while (true) {
        int local_count = 0;

        queue_lock.lock();

        // If there are no more nodes to process, then we will never have more
        // work to do.
        if (outstanding_nodes == 0) {
            queue_lock.unlock();
            break;
        }

        // If the queue is empty, sleep until it isn't. We wake up with the lock again.
        // TODO use condition variable to do this.

        // The queue may now have work, but if another thread woke up first and consumed all of it,
        // then we will just loop around and go back to sleep.

        // TODO: Arbitary constant
        // Collect a batch of nodes of a reasonable size. This reduces locking overhead. The batch
        // should be big enough to avoid overhead of tiny nodes, but small enough to ensure that
        // other threads can do work in parallel.
        while (!node_queue.empty() && local_count < 20000) {
            KDBuilderQueueNode *node = node_queue.dequeue();
            local_queue.enqueue(node);
            local_count += node->triangles.size();
        }

        // TODO: Instead of waking up everybody, it may be better to wake up another thread if there
        // is more in the queue. We'd probably have to know how many threads are waiting.

        queue_lock.unlock();

        // Process the batch we collected. When we're done, go back to either
        // immediately grab another batch or go to sleep.

        while (!local_queue.empty()) {
            KDBuilderQueueNode *node = local_queue.dequeue();

    		buildNode(ctx, node);

    		// Decrement reference count of parent. If we are the last child node to finish,
    		// delete the parent and the memory containing its triangle list. The root node
    		// doesn't have a parent.
    		if (node->parent && --node->parent->refCount == 0)
    			delete node->parent;

    		// Note: This must happen after the node is built, to ensure that its children
    		// are enqueued.
    		outstanding_nodes--;
        }
	}
}

KDTree *KDBuilder::build(const std::vector<Triangle> & triangles) {
	Timer timer;

	std::cout << "Building KD tree" << std::endl;

	KDBuilderQueueNode *q_node = new KDBuilderQueueNode();

	// Use pointers while building the tree, because we need to be able to sort, etc.
	// Triangles are converted to "setup" triangles, so we don't actually need the
	// triangle data anyway.
	q_node->triangles.reserve(triangles.size());

	for (auto & it : triangles)
		q_node->triangles.push_back(const_cast<Triangle *>(&it));

	q_node->bounds = buildAABB(q_node->triangles);
	q_node->depth = 0;
	q_node->node = new KDNode();
	q_node->parent = nullptr;

	// Reference count of 3 so that we can access its members after construction has
	// finished.
	q_node->refCount = 3;

	outstanding_nodes = 1;
    node_queue.enqueue(q_node);

	int num_threads = std::thread::hardware_concurrency();

	std::vector<std::thread> workers;

	for (int i = 0; i < num_threads; i++)
		workers.push_back(std::thread(std::bind(&KDBuilder::worker_thread, this)));

	std::cout << "Started " << num_threads << " worker threads" << std::endl;

	for (auto & worker : workers)
		worker.join();

	workers.clear();

	// TODO: start worker threads
	// TODO: join worker threads

    float elapsed = timer.getElapsedMilliseconds() / 1000.0;
    float cpu     = timer.getCPUTime() / 1000.0;

    printf("Done: %f seconds (total), %f seconds (CPU), speedup: %.02f\n",
        elapsed, cpu, cpu / elapsed);

    KDTree *tree = new KDTree(q_node->node, q_node->bounds);

	assert(--q_node->refCount == 0);
	delete q_node;

	return tree;
}
