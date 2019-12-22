#include "j1App.h"
#include "j1MainMenu.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1FadeToBlack.h"
#include "j1Audio.h"
#include "j1Scene.h"
#include "p2Log.h"


j1MainMenu::j1MainMenu()
{
	name.create("gui");

}

j1MainMenu::~j1MainMenu()
{}

bool j1MainMenu::Awake(pugi::xml_node& conf)
{
	LOG("Loading GUI atlas");
	bool ret = true;

	background_image = conf.child("background").attribute("file").as_string("");

	return ret;
}

bool j1MainMenu::Start()
{
	LOG("Loading background assets");
	bool ret = true;
	introTexture = App->tex->Load(background_image.GetString());

	SDL_Rect rec;
	rec.x = 416;
	rec.y = 172;
	rec.w = 218;
	rec.h = 58;

	LOG("START MAIN MENU");

	App->audio->PlayMusic("audio/music/main_menu_music.ogg");

	App->render->camera.x = 0;
	App->render->camera.y = 0;

	LOG("MAIN MENU");

	return ret;
}

bool j1MainMenu::CleanUp()
{

	LOG("Unloading main menu");
	App->tex->UnLoad(graphics);
	App->tex->UnLoad(introTexture);
	App->audio->CleanUp();

	return true;
}

bool j1MainMenu::Update(float dt)
{

	App->render->Blit(App->main_menu->introTexture, 0, 0);
	// TODO 2: make so pressing SPACE the KEN stage is loaded
	if (App->input->GetKey(SDL_SCANCODE_LEFT)==KEY_DOWN)
	{
		App->fade->FadeToBlack(App->main_menu, App->scene, 2);

	}

	return true;
}