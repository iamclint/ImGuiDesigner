#include "Textures.h"
#include "textures\button.embed"
#include "textures\check.embed"
#include "textures\child.embed"
#include "textures\combo.embed"
#include "textures\dragdrop.embed"
#include "textures\help.embed"
#include "textures\input.embed"
#include "textures\list.embed"
#include "textures\menu.embed"
#include "textures\radio.embed"
#include "textures\selectable.embed"
#include "textures\separator.embed"
#include "textures\slider.embed"
#include "textures\switch.embed"
#include "textures\tab.embed"
#include "textures\tabs.embed"
#include "textures\text.embed"
#include "textures\texture.embed"
#include "textures\theme.embed"
#include "textures\tree.embed"
#include "textures\widget.embed"
#include "textures\widget2.embed"

Textures::Textures()
{

}
void Textures::init()
{
	//most of the icons are from https://icons8.com
	images["button"] = new Walnut::Image(embedded::button, sizeof(embedded::button));
	images["check"] = new Walnut::Image(embedded::check, sizeof(embedded::check));
	images["child"] = new Walnut::Image(embedded::child, sizeof(embedded::child));
	images["combo"] = new Walnut::Image(embedded::combo, sizeof(embedded::combo));
	images["dragdrop"] = new Walnut::Image(embedded::dragdrop, sizeof(embedded::dragdrop));
	images["help"] = new Walnut::Image(embedded::help, sizeof(embedded::help));
	images["input"] = new Walnut::Image(embedded::input, sizeof(embedded::input));
	images["list"] = new Walnut::Image(embedded::list, sizeof(embedded::list));
	images["menu"] = new Walnut::Image(embedded::menu, sizeof(embedded::menu));
	images["radio"] = new Walnut::Image(embedded::radio, sizeof(embedded::radio));
	images["separator"] = new Walnut::Image(embedded::separator, sizeof(embedded::separator));
	images["slider"] = new Walnut::Image(embedded::slider, sizeof(embedded::slider));
	images["switch"] = new Walnut::Image(embedded::switch_app, sizeof(embedded::switch_app));
	images["tab"] = new Walnut::Image(embedded::tab, sizeof(embedded::tab));
	images["tabs"] = new Walnut::Image(embedded::tabs, sizeof(embedded::tabs));
	images["texture"] = new Walnut::Image(embedded::texture, sizeof(embedded::texture));
	images["theme"] = new Walnut::Image(embedded::theme, sizeof(embedded::theme));
	images["tree"] = new Walnut::Image(embedded::tree, sizeof(embedded::tree));
	images["widget"] = new Walnut::Image(embedded::widget, sizeof(embedded::widget));
	images["widget2"] = new Walnut::Image(embedded::widget2, sizeof(embedded::widget2));
	images["text"] = new Walnut::Image(embedded::text, sizeof(embedded::text));
	images["selectable"] = new Walnut::Image(embedded::selectable, sizeof(embedded::selectable));

}
Textures::~Textures()
{

}