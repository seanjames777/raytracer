/*
 * Sean James
 *
 * main.cpp
 *
 * Raytracer main method
 *
 */

#include "main.h"

#include "alignedlist.h"

int main(int argc, char *argv[]) {
	stringstream ss;
	ss << "logs/output_" << Timer::getCurrentTimeSeconds() << ".log";
	//Loggers::Main.setOutput(ss.str());

	/*Vec3 camp = Vec3(700, 700, 700);
	Vec3 camTarg = Vec3(0.0f, 473, 0.0f);*/

	Vec3 camp = Vec3(750, 750, 750);
	Vec3 camTarg = Vec3(0.0f, 250.0f, 0.0f);

	float d = (camTarg - camp).len();
	
	Bitmap bmp(1000, 1000, S_DEFAULT);
	bmp.save("output.bmp");

	Camera camera(camp, camTarg, (float)bmp.getWidth() / (float)bmp.getHeight(), (float)M_PI / 3.0f);

	GLImageDisplay app(bmp.getWidth(), bmp.getHeight(), &bmp);
	app.load();
	app.refresh();

	Raytracer rt;
	rt.setCamera(&camera, 0.8f, d);

	PhongSurface p1(Color(0.2f, 0.2f, 0.2f), Color(0.8f, 0.8f, 0.8f), Color(0.0f, 0.0f, 0.0f), 16, 0.0f, 0.0f, 0.0f, NULL, Vec2(1.0f, 1.0f));
	
	PlaneShape f1(Vec3(0, 1, 0), Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 0, 1), 20, 20);
	//rt.addShape(&f1);
	rt.setSurface(&f1, &p1);

	SphereShape s1(Vec3(4.0f, 2.0f, 0.0f), 4.0f);
	//rt.addShape(&s1);
	rt.setSurface(&s1, &p1);

	SphereShape s2(Vec3(-4.0f, 2.0f, 0.0f), 4.0f);
	//rt.addShape(&s2);
	rt.setSurface(&s2, &p1);

	DirectionalLight d1(Vec3(-1, -0.8f, 0.5f), Color(1.0f, 1.0f, 1.0f), true);
	rt.addLight(&d1);

	AlignedList<PolygonShape> polys((size_t)16);
	AABB polyBox;

	parseObj("bridge.obj", polys, &polyBox);

	for (int i = 0; i < polys.size(); i++) {
		rt.addShape(&polys[i]);
		rt.setSurface(&polys[i], &p1);
	}
	
	rt.setOutput(&bmp);
	rt.render(16, &app);

	bmp.save("output.bmp");

	getchar();

	app.unload();

	Loggers::Main.Log(LOGGER_CRITICAL, "Program terminating\n");
    return 0;
}