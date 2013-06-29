-- format a position
fn formatPos pos = (
	x = (-pos.x) as string
	z = pos.y as string
	y = pos.z as string
	 return "<" + x + ", " + y + ", " + z + ">"
)

-- format a color
fn formatColor color = (
	r= (color.r / 255.0) as string
	g = (color.g / 255.0) as string
	b = (color.b / 255.0) as string
	
	return "<" + r + ", " + g + ", " + b + ">"
)

--format a number
fn formatNum num = (
	return num as string
)

-- format a target camera
fn formatTargetCamera cam = (
	pos = formatPos cam.pos
	targ = formatPos cam.target.pos
	
	return "camera type: target, position: " + pos + ", target: " + targ
)

-- print all cameras in the scene
fn evalCameras = (
	for camera in $cameras do (
		type = classOf(camera)
		
		if (type == TargetCamera) do (
			print (formatTargetCamera camera)
		)
	)
)

-- format an omnidirectional light
fn formatOmniLight light = (
	pos = formatPos light.pos
	col = formatColor (light.color * light.multiplier)
	
	return "light type: point, position: " + pos + ", color: " + col
)

fn formatDirectionalLight light = (
	dir = formatPos light.dir
	col = formatColor (light.color * light.multiplier)
	
	return "light type: directional, direction: " + dir + ", color: " + col
)

-- print all lights in the scene
fn evalLights = (
	for light in $lights do (
		type = classOf(light)
		
		if (type == OmniLight) then (
			print (formatOmniLight light)
		)
		else if (type == TargetDirectionalLight) do (
			print (formatDirectionalLight light)
		)
	)
)

fn formatSphere sphere = (
	pos = formatPos sphere.pos
	rad = formatNum sphere.radius
	
	return "shape type: sphere, position: " + pos + ", radius: " + rad
)

fn formatPlane plane = (
	pos = formatPos plane.pos
	norm = formatPos ([ 0.0, 0.0, 1.0 ] * plane.rotation)
	w = formatNum plane.width
	l = formatNum plane.length
	
	return "shape type: plane, position: " + pos + ", normal: " + norm + ", width: " + w + ", length: " + l
)

fn evalGeometry = (
	for geom in $geometry do (
		type = classof(geom)
		
		if (type == Sphere) then (
			print (formatSphere geom)
		)
		else if (type == Plane) do (
			print (formatPlane geom)
		)
	)
)

evalCameras()
evalLights()
evalGeometry()