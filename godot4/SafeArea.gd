class_name SafeArea
extends RefCounted

# getRoundedCornerRadii() corner
# [top-left, top-right, bottom-right, bottom-left].
const CORNER_TOP_LEFT := 0
const CORNER_TOP_RIGHT := 1
const CORNER_BOTTOM_RIGHT := 2
const CORNER_BOTTOM_LEFT := 3


static func corner_radius(window: Window, corner: int) -> float:
	if OS.get_name() != "Android" or not Engine.has_singleton("FreeOrion"):
		return 0.0
	var radii: Array = Engine.get_singleton("FreeOrion").getRoundedCornerRadii()
	return maxf(float(radii[corner]) / window.content_scale_factor, 0.0)


# Margins between the window and the display safe area
# []left, top, right, bottom].
static func margins(window: Window) -> Vector4:
	var safe := DisplayServer.get_display_safe_area()
	var size := DisplayServer.window_get_size()
	var margin := Vector4(
		maxf(float(safe.position.x), 0.0),
		maxf(float(safe.position.y), 0.0),
		maxf(float(size.x - safe.end.x), 0.0),
		maxf(float(size.y - safe.end.y), 0.0),
	)
	return margin / window.content_scale_factor


# 45 degree margins from corner axis
static func corner_inset(radius: float) -> float:
	return radius * (1.0 - sqrt(2.0) / 2.0)


# margin from one corner axis with fixed other axis
static func corner_clearance(radius: float, offset: float) -> float:
	if radius <= 0.0 or offset >= radius:
		return 0.0
	var distance := radius - offset
	return radius - sqrt(radius * radius - distance * distance)
