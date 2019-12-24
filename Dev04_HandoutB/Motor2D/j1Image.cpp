#include "j1Image.h"

j1Image::j1Image(int x, int y, SDL_Rect image_rect, UI_element* parent, j1Module* CallBack) : UI_element(x, y, parent, CallBack)
{
	dimensions = image_rect;
}

j1Image::~j1Image() {}

bool j1Image::Draw() {
	
	texture = App->gui->GetAtlas();

	if (texture != nullptr)
	{
		App->render->Blit(texture, pos.x, pos.y, &dimensions, SDL_FLIP_NONE, 1.0f);
	}
	return true;
}