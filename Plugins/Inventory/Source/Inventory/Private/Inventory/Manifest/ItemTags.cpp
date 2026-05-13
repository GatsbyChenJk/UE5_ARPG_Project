#include "Inventory/Manifest/ItemTags.h"

namespace GameItems
{
	namespace Equipment
	{
		namespace Weapons
		{
			UE_DEFINE_GAMEPLAY_TAG(Axe,"GameItems.Equipment.Weapons.Axe")
			UE_DEFINE_GAMEPLAY_TAG(Sword,"GameItems.Equipment.Weapons.Sword")
		}

		namespace Cloaks
		{
			UE_DEFINE_GAMEPLAY_TAG(RedCloak,"GameItems.Equipment.Cloaks.RedCloak")
		}

		namespace Masks
		{
			UE_DEFINE_GAMEPLAY_TAG(SteelMask,"GameItems.Equipment.Masks.SteelMask")
		}

		namespace Armors
		{
			UE_DEFINE_GAMEPLAY_TAG(SmallShield1,"GameItems.Equipment.Armors.SmallShield1")
			UE_DEFINE_GAMEPLAY_TAG(MiddleShield1,"GameItems.Equipment.Armors.MiddleShield1")
			UE_DEFINE_GAMEPLAY_TAG(LargeShield1,"GameItems.Equipment.Armors.LargeShield1")
		}
	}

	namespace Comsumable
	{
		namespace Portion
		{
			namespace Red
			{
				UE_DEFINE_GAMEPLAY_TAG(Small,"GameItems.Comsumable.Portion.Red.Small")
				UE_DEFINE_GAMEPLAY_TAG(Large,"GameItems.Comsumable.Portion.Red.Large")
			}

			namespace Blue
			{
				UE_DEFINE_GAMEPLAY_TAG(Small,"GameItems.Comsumable.Portion.Blue.Small")
				UE_DEFINE_GAMEPLAY_TAG(Large,"GameItems.Comsumable.Portion.Blue.Large")
			}
		}
	}

	namespace Craftable
	{
		UE_DEFINE_GAMEPLAY_TAG(FireFernFruit,"GameItems.Craftable.FireFernFruit")
		UE_DEFINE_GAMEPLAY_TAG(LuminDaisy,"GameItems.Craftable.LuminDaisy")
		UE_DEFINE_GAMEPLAY_TAG(ScorchPetalBlossom,"GameItems.Craftable.ScorchPetalBlossom")
	}
}
