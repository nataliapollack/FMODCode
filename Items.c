// ---------------------------------------------------------------------------
// File Name		:	Items.c
// Project Name     :   DigiPocalypse
// Author			:	Natalia Pollack - Primary
//						Raj Saini - Inventory sorting and shifting
// Creation Date	:	2/13/2020
// Purpose			:	Interactable items for the player
// All content © 2020 DigiPen (USA) Corporation, all rights reserved.
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// includes
#include "stdafx.h"
#include "Trace.h"
#include "AEEngine.h"
#include "Mesh.h"
#include "Items.h"
#include "Camera.h"
#include "Player.h"
#include "Audio.h"
#include "Dialog.h"
#include "Collision.h"
#include "GameStateManager.h"
#include "Objective.h"
#include "SaveData.h"
#include "Map.h"
// ---------------------------------------------------------------------------

static int currentItems = 0; // How many items have been picked up as of now?

Mesh ItemSlot;
Texture ItemTexture;

Mesh AboutItem;
Texture AboutTexture;

Message* PickingUp;

//messages 4 the items
Message* Item;

// for the inventory
ItemsPtr Inventory[6];

//for which item is being info
int InfoItemNumber;

//for displaying message
bool DisplayingMessage = false;

// ---------------------------------------------------------------------------
// Private Struct
typedef struct Items
{
	Mesh mesh;               // Item Mesh

	Texture texture;         // Item Texture

	BoundingBox* boundingbox; // collision + bounding box

	bool hasItem;             // Flag if player has Item

	float xPos;				  // position of the item in x

	float yPos;				  // position of the item in y

	float width;			// width of the item

	float height;				// height of the item

	int itemNumber;			// to identify which item is which

	int index; // Index of this item in the array

	bool isUsed;			// Whether the item was used

	GameState state;		// The game state in which this item needs to be drawn (when not in the slot)

}Items;
// ---------------------------------------------------------------------------

// to make items smaller to store in the item slot
void ScaleItem(ItemsPtr item)
{
	item->mesh = createRect(0, 0, 60, 60);
}

//adds an item to the item slot
void ItemSlotAdd(ItemsPtr item)
{
	if (InventoryFull())
		return;

	int i;
	for (i = 0; i < 6; i++)
	{
		if (!Inventory[i])
		{
			Inventory[i] = item;
			if (item->hasItem)
				ScaleItem(item);
			break;
		}
	} 
}

ItemsPtr ItemsCreate(float x, float y, float w, float h, const char* filename, int itemNumber_, GameState state)
{
	// checking if its in the inventory
	if (InInventory(itemNumber_))
		return NULL;

	ItemsPtr items = (ItemsPtr)calloc(1, sizeof(Items));

	if (items != NULL)
	{
		items->xPos = x;
		items->yPos = y;
		items->width = w;
		items->height = h;

		items->itemNumber = itemNumber_;

		items->mesh = createRect(0, 0, w, h);
		items->texture = AEGfxTextureLoad(filename);
		items->boundingbox = BoundingBoxCreate(x + 35.f, y - 200.0f, w - 100.f, 600.0f);
		items->hasItem = false;
		items->isUsed = false;
		items->state = state;

		ItemSlotAdd(items);

		return items;
	}
	return NULL;
}

//item slot in it
void ItemSlotInit()
{
	PickingUp = MessageCreate(0, 0, 30, "Press 'Z' to pick up", false);

	ItemTexture = AEGfxTextureLoad("./Assets/items/itemSlot.png");
	ItemSlot = createRect(0 ,0, 425, 70);

	InfoItemNumber = cItemInvalid;

	MapLoad();
	
	int i;
	for (i = 0; i < 6; ++i);
		Inventory[i] = NULL;
}


ItemsPtr ReturnItem(int itemnumber)
{
	int i;
	for (i = 0; i < 6; ++i)
	{
		if (Inventory[i] != NULL)
			if (Inventory[i]->itemNumber == itemnumber)
				return Inventory[i];
	}
	return NULL;
}

// Updates the position the item gets drawn at
void ItemUpdatePosition(float xVel, float yVel, ItemNumber item)
{
	ItemsPtr itemToUpdate = ReturnItem(item);

	if (itemToUpdate)
	{
		itemToUpdate->xPos += xVel;
		itemToUpdate->yPos += yVel;

		if (itemToUpdate->yPos < -150)
			itemToUpdate->yPos = -150;

		BoundingBoxSetPosition(itemToUpdate->boundingbox, itemToUpdate->xPos + 35.0f, itemToUpdate->yPos - 200.0f);
	}
}

// writes the message 4 the items
Message* WriteThis(int whichitem)
{
	ItemsPtr item = ReturnItem(whichitem);

	if (item)
	{
		// No messages will b written if u don't have the item
		if (item->isUsed || !item->hasItem)
			return NULL;

		switch (whichitem)
		{
		case CSKey:
			Item = MessageCreate(CameraGetX(), CameraGetY(), 30, "It's Rohit Saini's missing key card. He was a BSGD. (Press 'Z'  to dismiss)", false);
			return Item;

		case Muffin:
			Item = MessageCreate(CameraGetX(), CameraGetY(), 30, "It's a department store bought muffin   that the school resells. (Press enter to use)", false);
			return Item;

		case Map:
			Item = MessageCreate(CameraGetX(), CameraGetY(), 30, "It's a map of the school. (Press enter  to use)", false);
			return Item;

		case ArtKey:
			Item = MessageCreate(CameraGetX(), CameraGetY(), 30, "It seems to be a long lost keycard of BFA... (Press 'Z'  to dismiss)", false);
			return Item;

		case GetOutKey:
			Item = MessageCreate(CameraGetX(), CameraGetY(), 30, "It is the key to leave this monstrous place. (Press 'Z'  to dismiss)", false);
			return Item;
		}
	}

	return NULL;
}

//helper function to set item positions in the item slot
void SetItemPosition(int i, ItemsPtr item)
{
	if (!item->hasItem) // Draw the item whereever it is if it hasn't been picked up
		return;

	switch (i)
	{
	case 0:
		//cameragetx + 200
		item->xPos = CameraGetX() + 200;
		//camergety + 250
		item->yPos = CameraGetY()  + 255;
		break;
	case 1:
		//camx + 270
		item->xPos = CameraGetX() + 270;
		item->yPos = CameraGetY() + 255;
		break;
	case 2:
		//camx + 340
		item->xPos = CameraGetX() + 340;
		item->yPos = CameraGetY() + 255;
		break;
	case 3:
		//camx + 410
		item->xPos = CameraGetX() + 410;
		item->yPos = CameraGetY() + 255;
		break;
	case 4:
		//camx + 480
		item->xPos = CameraGetX() + 480;
		item->yPos = CameraGetY() + 255;
		break;
	case 5:
		//camx + 550
		item->xPos = CameraGetX() + 550;
		item->yPos = CameraGetY()  + 255;
		break;

	}
}

//draws item slot
void ItemSlotDraw(GameState currentState)
{
	int i;
	float x;

	for (x = 200.0f, i = 0; i < 6; ++i, x += 50)
	{
		if (Inventory[i])
		{
			SetItemPosition(i, Inventory[i]);

			if (!Inventory[i]->isUsed) // Only draw unused items
				if (Inventory[i]->hasItem || (!Inventory[i]->hasItem && Inventory[i]->state == currentState))
				{
					drawMesh(Inventory[i]->xPos, Inventory[i]->yPos, Inventory[i]->mesh, Inventory[i]->texture, 1.f, AE_GFX_RM_TEXTURE, 0.f, 0.f, 0.f, 0.f);
				}
		}
	}

	x = 200.0f;
	drawMesh(CameraGetX() + x, CameraGetY() + 250, ItemSlot, ItemTexture, 1, AE_GFX_RM_TEXTURE, 0, 0, 0, 0);

	//update the map
	MapUpdate(0);
}

//checks for which button is being pressed
int ButtonCheck()
{
	if (AEInputCheckCurr('1'))
	{
		if (Inventory[0])
			return Inventory[0]->itemNumber;

		return cItemInvalid;
	}

	else if (AEInputCheckCurr('2'))
	{
		if (Inventory[1])
		{
			return Inventory[1]->itemNumber;
		}

		return cItemInvalid;
	}

	else if (AEInputCheckCurr('3'))
	{
		if (Inventory[2])
		{
			return Inventory[2]->itemNumber;
		}
		return cItemInvalid;
	}

	else if (AEInputCheckCurr('4'))
	{
		if (Inventory[3])
		{
			return Inventory[3]->itemNumber;
		}
		return cItemInvalid;
	}

	else if (AEInputCheckCurr('5'))
	{
		if (Inventory[4])
		{
			return Inventory[4]->itemNumber;
		}
		return cItemInvalid;
	}

	else if (AEInputCheckCurr('6'))
	{
		if (Inventory[5])
		{
			return Inventory[5]->itemNumber;
		}
		return cItemInvalid;
	}

	else
	{
		InfoItemNumber = cItemInvalid;
		return cItemInvalid;
	}

}


//returns the item x value
float ReturnItemX(int itemnumber)
{
	int i;
	for (i = 0; i < 6; ++i)
	{
		if (Inventory[i])
		{
			if (Inventory[i]->itemNumber == itemnumber)
				return Inventory[i]->xPos;
		}
	}
	return 0;
}

ItemsPtr UseItem(ItemsPtr item)
{
	if (item)
	{
		if (item->itemNumber == Muffin)
		{
			PlayerEatMuffin();
			item->isUsed = true; // Mark item as consumed or used for consumable only
		}

		if (item->itemNumber == CSKey)
		{
			// Nothing is done when u press enter on the CSKey
		}

		if (item->itemNumber == Map)
		{
			//when u press enter on the Map, show the map
			setUsingMap(true);
		}

		if (item->itemNumber == GetOutKey)
		{
			//do nothing when u use the get out key
		}

		return item;
	}

	return NULL;
}

void MoveItOver(int i)
{
	int j = 5;
	for (; j > 0; j--)
	{
		if (Inventory[j] == NULL)
		{
			Inventory[j] = Inventory[i];
			Inventory[i] = NULL;
			break;
		}
	}

	// Now move all items from the front to the left (unused ones)
	j = 0;
	for (; j < 6; j++)
	{
		if (Inventory[j] == NULL)
		{
			int k = j + 1;
			for (; k < 6; k++)
			{
				if (Inventory[k] != NULL)
					if (!Inventory[k]->isUsed)
					{
						Inventory[j] = Inventory[k];
						Inventory[k] = NULL;
						break;
					}
			}
		}
	}
	DisplayingMessage = false;
}

void RemoveItem(ItemsPtr item)
{
	if (!item) // Nothing to remove
		return;

	int i;
	for (i = 0; i < 6; ++i)
	{
		if (Inventory[i])
		{
			if (Inventory[i]->itemNumber == item->itemNumber)
			{
				ItemsFreeItem(&Inventory[i]);
				break;
			}
		}
	}
}

// tells player about the info
void ItemInfo()
{
	switch (ButtonCheck())
	{
		case cItemInvalid:
			InfoItemNumber = cItemInvalid;
			DisplayingMessage = false;
			break;

		case CSKey:
			InfoItemNumber = CSKey;
			DisplayingMessage = true;
			NatsFivePercent(WriteThis(InfoItemNumber));
			break;
		case Muffin:
			InfoItemNumber = Muffin;
			DisplayingMessage = true;
			NatsFivePercent(WriteThis(InfoItemNumber));
			break;
		case Map:
			InfoItemNumber = Map;
			DisplayingMessage = true;
			NatsFivePercent(WriteThis(InfoItemNumber));
			break;
		case ArtKey:
			InfoItemNumber = ArtKey;
			DisplayingMessage = true;
			NatsFivePercent(WriteThis(InfoItemNumber));
			break;
		case GetOutKey:
			InfoItemNumber = GetOutKey;
			DisplayingMessage = true;
			NatsFivePercent(WriteThis(InfoItemNumber));
			break;
	}

	if (DisplayingMessage)
	{
		NatsFivePercent(WriteThis(InfoItemNumber));

		//If they hit z make it go away
		if (AEInputCheckTriggered('Z'))
			DisplayingMessage = false;

		else if (AEInputCheckTriggered(VK_RETURN))
		{
			DisplayingMessage = false;
			if (InfoItemNumber == Muffin)
			{
				ItemsPtr muffin = ReturnItem(Muffin);

				UseItem(muffin);
				DisplayingMessage = false;

				MoveItOver(muffin->index);
			}

			if (InfoItemNumber == CSKey)
			{
				ItemsPtr key = ReturnItem(CSKey);

				UseItem(key);
				DisplayingMessage = false;

			//	MoveItOver(key->index);
			}

			if (InfoItemNumber == Map)
			{
				ItemsPtr map = ReturnItem(Map);

				UseItem(map);
				DisplayingMessage = false;

			//	MoveItOver(map->index);
			}

			if (InfoItemNumber == ArtKey)
			{
				ItemsPtr artkey = ReturnItem(ArtKey);

				UseItem(artkey);
				DisplayingMessage = false;

			//	MoveItOver(artkey->index);
			}

			if (InfoItemNumber == GetOutKey)
			{
				ItemsPtr getoutkey = ReturnItem(GetOutKey);

				UseItem(getoutkey);
				DisplayingMessage = false;

				//	MoveItOver(artkey->index);
			}
		}
	}

	if (AEInputCheckCurr('Z'))
	{
		InfoItemNumber = cItemInvalid;

		DisplayingMessage = false;
	}
}


void ItemsFreeItem(ItemsPtr* item)
{
	if (*item == NULL)
		return;

	if (item)
	{
		AEGfxMeshFree((*item)->mesh);
		AEGfxTextureUnload((*item)->texture);

		BoundingBoxFree((*item)->boundingbox);

		free(*item);
		*item = NULL;
		item = NULL;
	}
}

//Free all items
void ItemsFreeAll()
{
	int i;
	for (i = 0; i < 6; i++)
		if (Inventory[i])
			ItemsFreeItem(&Inventory[i]);
}

int HasItem(ItemsPtr item)
{
	if (item != NULL)
	  return item->hasItem;

	return false;
}

// checks for if item is in the players inventory
bool InInventory(int itemnumber)
{
	int i;
	for (i = 0; i < 6; ++i)
	{
		if (Inventory[i])
		{
			//they can have more than 1 muffin
			if (Inventory[i]->itemNumber == itemnumber && itemnumber != Muffin)
				return true;
		}
	}
	return false;
}

void ItemsDraw(ItemsPtr item, GameState currentState)
{
	if (item->hasItem || item->isUsed || (item->state != currentState))
		return;

	if (item != NULL)
	  drawMesh(item->xPos, item->yPos, item->mesh, item->texture, 1.f, AE_GFX_RM_TEXTURE, 0.f, 0.f, 0.f, 0.f);
}

void ItemsUpdateAll(GameState currentState)
{
	int i;
	for (i = 0; i < 6; i++)
		if (Inventory[i] != NULL)
		{
			if (!Inventory[i]->isUsed)
				ItemsUpdate(Inventory[i], currentState);
		}

	ItemSlotDraw(currentState); // Draw everything
}

// Swap the item at the new item index with the 'current' index
void ItemsSort(int newItemIndex)
{
	if (Inventory[currentItems] != NULL)
	{
		ItemsPtr temp = Inventory[currentItems];
		Inventory[currentItems] = Inventory[newItemIndex];
		Inventory[newItemIndex] = temp;
	}
}

bool InventoryFull()
{
	return currentItems >= 6;
}

bool tooManyMuffins()
{
	int i;
	int nMuffins = 0;

	for (i = 0; i < 6; i++)
		if (Inventory[i] != NULL)
		{
			if (Inventory[i]->itemNumber == Muffin)
				nMuffins++;
		}

	if (nMuffins >= 1)
		return true;
	else
		return false;
}

void AddMuffinToInventory()
{
	ItemsPtr item = ItemsCreate(-700, 0, 100, 100, "./Assets/items/muffin.png", Muffin, GsStore);

	item->hasItem = true;

	item->index = currentItems; // Initial index is 0, then 1, ...

	currentItems++; // Set what the index of the item will be

	ItemsSort(item->index);

	ScaleItem(item);

	PlayGSSoundEffect(gsinteraction);
}

void ItemsUpdate(ItemsPtr item, GameState currentState)
{
	if (item == NULL)
		return;

	if (item)
	{
		ItemsDraw(item, currentState);
		ItemInfo();
		if (CollisionCheckWith(item->boundingbox))
		{

			if (!item->hasItem && item->state == currentState)
			{
				//display message 'Z' to pick up
				if (!PickingUp)
					PickingUp = MessageCreate(0, 0, 30, "Press 'Z' to pick up", false);
				MessageSetPosition(PickingUp, item->xPos, item->yPos - 40);
				DisplayMessage(PickingUp);
			}

			if (AEInputCheckTriggered('Z') && item->state == currentState)
			{
				if (!item->hasItem)
				{
					//the inventory is full, do nothing
					if (currentItems >= 6)
					{
						ResetAllDialogs();
						DialogSetPosition(CameraGetX() - 300, 150, false);
						DisplayText("Inventory seems to be full...");
						DialogSetTextSize(20.0f);
						return;
					}

					item->hasItem = true;

					item->index = currentItems; // Initial index is 0, then 1, ...

					currentItems++; // Set what the index of the item will be

					ItemsSort(item->index);

					ScaleItem(item);
					// ItemSlotAdd(item);
					PlayGSSoundEffect(gsinteraction);
					
					if (item->itemNumber == CSKey)
						if (ObjectiveIs(ObjectiveEncounterDubatron))
						{
							ResetAllDialogs();
							DialogSetPosition(CameraGetX() - 300, 150, false);
							DisplayText("It seems to be some sort of keycard...");
							DialogSetTextSize(20.0f);
							AdvanceObjective();

							// Now spawn the map
							ItemsCreate(460, -140, 90.0f, 90.0f, "./Assets/items/map.png", Map, GsMichelangelo);
						}

					if (item->itemNumber == Map)
						if (ObjectiveIs(ObjectiveFindMap))
						{
							ResetAllDialogs();
							DialogSetPosition(CameraGetX() - 300, 150, false);
							DisplayText("It's an old map of DigiPen...");
							DialogSetTextSize(20.0f);
							AdvanceObjective();

							// Now spawn the next item if required
						}

					if (item->itemNumber == ArtKey)
						if (ObjectiveIs(ObjectiveFindArtKey))
						{
							ResetAllDialogs();
							DialogSetPosition(CameraGetX() - 300, 150, false);
							DisplayText("It seems to be another keycard...");
							DialogSetTextSize(20.0f);
							AdvanceObjective();

							//save the data so far
							SaveData();

							// Now spawn the next item (get out key)
							ItemsCreate(600, 270, 90, 90, "./Assets/items/key.png", GetOutKey, GsArtRoom);
						}

					if (item->itemNumber == GetOutKey)
						if (ObjectiveIs(ObjectiveEncounterClayBoss))
						{
							ResetAllDialogs();
							DialogSetPosition(CameraGetX() - 300, 150, false);
							DisplayText("It seems to be the key to exit the building...");
							DialogSetTextSize(20.0f);

							AdvanceObjective();

						}
				}
			}
		}
	}
}