#include "oxygine-framework.h"
#include <functional>
#include "Input.h"
#include "SDL.h"
#include "SDL_keyboard.h"
#include <vector>
#include <sstream>
#include <string>
#include "TMXParser.h"
using namespace oxygine;

//it is our resources
//in real project you would have more than one Resources declarations.
//It is important on mobile devices with limited memory and you would load/unload them
Resources gameResources;

class SpriteButton : public Sprite
{
protected:
	void doUpdate(const UpdateState &us) override
	{
		const Uint8* data = SDL_GetKeyboardState(0);
		float speed = 100.0f * (us.dt / 1000.0f);
		Vector2 pos = getPosition();

		if (data[SDL_SCANCODE_A]) pos.x -= speed;
		if (data[SDL_SCANCODE_D]) pos.x += speed;
		if (data[SDL_SCANCODE_W]) pos.y -= speed;
		if (data[SDL_SCANCODE_S]) pos.y += speed;

		setPosition(pos);
	}
};
typedef oxygine::intrusive_ptr<SpriteButton> spSpriteButton;

class TileMap : public Sprite
{
private:
	size_t m_rows = 0;
	size_t m_cols = 0;
	size_t m_tileRows = 0;
	size_t m_tileColoumns = 0;
	spResAnim m_resource;
	std::vector<spSprite> m_tileSprites;
public:
	TileMap(size_t rows, size_t cols, size_t tileCount, size_t tileColoumns, const std::string& resourceName) 
		:	m_rows(rows)
		,	m_cols(cols)
		,	m_resource(gameResources.getResAnim(resourceName))
		,	m_tileRows(tileCount / tileColoumns)
		,	m_tileColoumns(tileColoumns)
	{
		m_tileSprites.reserve(rows * cols);
	}
	
	void SetTiles(const std::string& tileDescriptor)
	{
		std::istringstream tileDescriptorStream(tileDescriptor);
		std::string currentTile;
		while (std::getline(tileDescriptorStream, currentTile, ','))
		{
			int tileIndex = std::stoi(currentTile) - 1;
			spSprite newTile = new Sprite;

			newTile->setResAnim(m_resource.get(), tileIndex % m_tileRows, tileIndex / m_tileRows);
			newTile->attachTo(this);
			m_tileSprites.emplace_back(std::move(newTile));
		}
	}
	void transformUpdated() override
	{
		auto currentPosition = Vector2(0, 0);
		auto tileSize = m_tileSprites[0]->getSize();
		size_t i = 0;
		for (size_t y = 0; y < m_rows; ++y, currentPosition.y += tileSize.y - 0.5)
		{
			currentPosition.x = 0;
			for (size_t x = 0; x < m_cols; ++x, ++i, currentPosition.x += tileSize.x - 0.5)
			{
				m_tileSprites[i]->setPosition(currentPosition);
			}
		}
	}
};

typedef oxygine::intrusive_ptr<TileMap> spTileMap;

class MainActor: public Actor
{

public:
    spTextField Text;
	spTileMap    Button;
	spSprite	 Map;
protected:
	void doUpdate(const UpdateState &us) override
	{
		const Uint8* data = SDL_GetKeyboardState(0);
		float speed = 100.0f * (us.dt / 1000.0f);
		auto posButton = Button->getPosition();
		auto posMap = Map->getPosition();

		if (data[SDL_SCANCODE_D])
		{
			if (posButton.x < getStage()->getWidth() - Button->getWidth() - 30)
				posButton.x += speed;
			else
				posMap.x -= speed;
		}

		if (data[SDL_SCANCODE_A])
		{
			if (posButton.x > Button->getWidth() + 30)
				posButton.x -= speed;
			else
				posMap.x += speed;
		}

		if (data[SDL_SCANCODE_S])
		{
			if (posButton.y < getStage()->getHeight() - Button->getHeight() - 30)
				posButton.y += speed;
			else
				posMap.y -= speed;
		}

		if (data[SDL_SCANCODE_W])
		{
			if (posButton.y > Button->getHeight() + 30)
				posButton.y -= speed;
			else
				posMap.y += speed;
		}

		Button->setPosition(posButton);
		Map->setPosition(posMap);

	}

public:

    MainActor()
    {
		Map = new Sprite();
		Map->setResAnim(gameResources.getResAnim("world_map"));
		Map->setPosition(Vector2(0, 0));
		addChild(Map);

		TMX::Parser tmx;
		tmx.load("test_map.tmx");
		
		;
        //create button Sprite
        /*spSprite*/ Button = new TileMap(5, 5, 256, 16, "PathAndObjects");
		Button->SetTiles(/*"1,2,2,2,3,\
			17,18,18,18,19,\
			17,18,18,18,19,\
			17,18,18,18,19,\
			33,34,34,34,35"*/tmx.tileLayer[tmx.tileLayer.begin()->first].data.contents);

        //setup it:
        //set button.png image. Resource 'button' defined in 'res.xml'
        //Button->setResAnim(gameResources.getResAnim("button"));

		

        //centered button at screen
        Vector2 pos = getStage()->getSize() / 2 - Button->getSize() / 2;
        Button->setPosition(pos);

        //register  click handler to button
        EventCallback cb = CLOSURE(this, &MainActor::buttonClicked);
        Button->addEventListener(TouchEvent::CLICK, cb);

        Button->addEventListener(TouchEvent::CLICK, [](Event * e)->void
        {
            log::messageln("button clicked");
        });
		
		//attach button as child to current actor
        addChild(Button);
        //_button = button;

        //create TextField Actor
        spTextField text = new TextField();
        //attach it as child to button
        text->attachTo(Button);
        //centered in button
        text->setPosition(Button->getSize() / 2);

        //initialize text style
        TextStyle style;
        style.font = gameResources.getResFont("main")->getFont();
        style.color = Color::White;
        style.vAlign = TextStyle::VALIGN_MIDDLE;
        style.hAlign = TextStyle::HALIGN_CENTER;

        text->setStyle(style);
        text->setText("Click\nMe!");

        Text = text;
    }



    void buttonClicked(Event* event)
    {
        //user clicked to button

        //animate button by chaning color
        //Button->setColor(Color::White);
        Button->addTween(Sprite::TweenColor(Color::Green), 500, 1, true);

        //animate text by scaling
        Text->setScale(1.0f);
        Text->addTween(Actor::TweenScale(1.1f), 500, 1, true);

        //and change text
        Text->setText("Clicked!");

        //lets create and run sprite with simple animation
        runSprite();
    }

    void runSprite()
    {
        spSprite sprite = new Sprite();
        addChild(sprite);

        int duration = 500;//500 ms
        int loops = -1;//infinity loops

        //animation has 7 columns, check 'res.xml'
        ResAnim* animation = gameResources.getResAnim("anim");

        //add animation tween to sprite
        //TweenAnim would change animation frames
        sprite->addTween(Sprite::TweenAnim(animation), duration, loops);

        Vector2 destPos = getStage()->getSize() - sprite->getSize();
        Vector2 srcPos = Vector2(0, destPos.y);
        //set sprite initial position
        sprite->setPosition(srcPos);

        //add another tween: TweenQueue
        //TweenQueue is a collection of tweens
        spTweenQueue tweenQueue = new TweenQueue();
        tweenQueue->setDelay(1500);
        //first, move sprite to dest position
        tweenQueue->add(Sprite::TweenPosition(destPos), 1500, 1);
        //then fade it out smoothly
        tweenQueue->add(Sprite::TweenAlpha(0), 500, 1);

        sprite->addTween(tweenQueue);

        //and remove sprite from tree when tweenQueue is empty
        //if you don't hold any references to sprite it would be deleted automatically
        tweenQueue->setDetachActor(true);
    }
};
//declare spMainActor as intrusive_ptr holder of MainActor
typedef oxygine::intrusive_ptr<MainActor> spMainActor;
//you could use DECLARE_SMART preprocessor definition it does the same:
//DECLARE_SMART(MainActor, spMainActor)

void example_preinit() {}

//called from entry_point.cpp
void example_init()
{
    //load xml file with resources definition
    gameResources.loadXML("res.xml");


    //lets create our client code simple actor
    //spMainActor was defined above as smart intrusive pointer (read more: http://www.boost.org/doc/libs/1_60_0/libs/smart_ptr/intrusive_ptr.html)
	spMainActor actor = new MainActor();

    //and add it to Stage as child
    getStage()->addChild(actor);
}




//called each frame from entry_point.cpp
void example_update()
{
}

//called each frame from entry_point.cpp
void example_destroy()
{
    //free previously loaded resources
    gameResources.free();
}
