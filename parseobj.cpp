/*
 * Sean James
 *
 * parseobj.cpp
 *
 * parseObj() function for parsing a .obj model into an array of PolygonShapes.
 *
 */

#include "parseobj.h"

/*
 * Parse the file specified by 'filename' as a .obj model. Sets the pointer
 * out_polys to an array of PolygonShapes, sets out_npolys to the number of polygons,
 * and sets out_bbox to a bounding box for the polygons. The caller is responsible
 * for freeing out_polys when done.
 *
 * Polys must be aligned to 16
 *
 */
bool parseObj(char *filename,  AlignedList<PolygonShape> & polys, AABB *out_bbox) {
		char word[1024];

	AlignedList<Vec3> positions(16), normals(16);
	AlignedList<Vec2> uvs(16);

	FILE *file = fopen(filename, "r");

	if (file == NULL)
		return false;

	Timer timer;
	timer.start();

	AABB box;
	bool boxInit = false;

	while(true) {
		int res = fscanf(file, "%s", word);

		if (res == EOF)
			break;

		if (strcmp(word, "v") == 0) {
			Vec3 position;
			fscanf(file, "%f %f %f\n", &position.x, &position.y, &position.z);
			positions.push(position);

			if (!boxInit) {
				box = AABB(position, position);
				boxInit = true;
			}
			else
				box.join(position);
		}
		else if (strcmp(word, "vt") == 0) {
			Vec2 uv;
			float w;
			fscanf(file, "%f %f %f\n", &uv.x, &uv.y, &w);
			uvs.push(uv);
		}
		else if (strcmp(word, "vn") == 0) {
			Vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normal.normalize();
			normals.push(normal);
		}
		else if (strcmp(word, "f") == 0) {
			int posIdx[3], uvIdx[3], normIdx[3];

			fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&posIdx[0], &uvIdx[0], &normIdx[0],
				&posIdx[1], &uvIdx[1], &normIdx[1],
				&posIdx[2], &uvIdx[2], &normIdx[2]);

			Vertex v1(positions[posIdx[0] - 1], normals[normIdx[0] - 1], uvs[uvIdx[0] - 1]);
			Vertex v2(positions[posIdx[1] - 1], normals[normIdx[1] - 1], uvs[uvIdx[1] - 1]);
			Vertex v3(positions[posIdx[2] - 1], normals[normIdx[2] - 1], uvs[uvIdx[2] - 1]);

			PolygonShape poly(v1, v2, v3);
			polys.push(poly);
		}
	}

	fclose(file);

	Loggers::Main.Log(LOGGER_NONCRITICAL, "%s:\n", filename);
	timer.stopAndPrint("OBJ Parse");
	Loggers::Main.Log(LOGGER_NONCRITICAL, "   %d positions\n", positions.size());
	Loggers::Main.Log(LOGGER_NONCRITICAL, "   %d normals\n",   normals.size());
	Loggers::Main.Log(LOGGER_NONCRITICAL, "   %d uvs\n",       uvs.size());
	Loggers::Main.Log(LOGGER_NONCRITICAL, "   %d polygons\n",  polys.size());

	if (out_bbox != NULL)
		*out_bbox = box;

	return true;
}
