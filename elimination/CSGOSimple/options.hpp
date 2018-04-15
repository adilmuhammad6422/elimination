#pragma once

#include <string>
#include <vector>
#include <stdio.h>
#include "valve_sdk/Misc/Color.hpp"
#include "valve_sdk\csgostructs.hpp"

#define OPTION(type, var, val) type var = val

extern bool meme;
extern QAngle LastTickViewAngles;
extern QAngle ChamFakeAngle;
extern bool aimstepInProgress;
extern bool sendPacket;
extern bool LowerBodyIsUpdated;
extern bool updatedColors;
extern bool aaSide;

enum Hitscanning_t
{
	HITSCAN_NONE,
	HITSCAN_LOW,
	HITSCAN_MEDIUM,
	HITSCAN_HIGH,
	HITSCAN_EXTREME // LAG WARNING (every bone possible in the player's hitbox)
};

static char* HitscanNames[] =
{
	"None",
	"Low",
	"Medium",
	"High",
	"As High As Me"
};

enum class SmoothType
{
	SLOW_END,
	CONSTANT,
	FAST_END
};

enum EspType_t
{
	ICONS,
	TEXT
};

enum Sky_t
{
	NO_SKYCHANGE,
	VIETNAM,
	VERTIGO,
	SKY_CSGO_NIGHT02,
	SKY_CSGO_NIGHT02B
};

enum Name_t
{
	KAWAII_EXPOSED,
	NO_NAME,
	NIGGER,
	CUTE,
	ORIGINAL
};

enum Clantag_t
{
	DISABLED_CLANTAG,
	NO_CLANTAG,
	ELIMINATION_STATIC,
	UFFYA,
	SILVER,
	BHOP,
	VALVE,
	SKEET,
	OWO,
	ANIMIATED_XD,
	STARS,

};

enum chatspam {
	chatspam_none,
	KAWAII_SPAM,
	TUMBLR_SPAM,
};

enum AntiAimYaw_t
{
	ANTIAIM_YAW_NONE,

	ANTIAIM_YAW_SPIN,
	ANTIAIM_YAW_STATIC_FORWARD,
	ANTIAIM_YAW_STATIC_RIGHT,
	ANTIAIM_YAW_STATIC_BACKWARDS,
	ANTIAIM_YAW_STATIC_LEFT,
	ANTIAIM_YAW_BACKWARDS,
	ANTIAIM_YAW_LEFT,
	ANTIAIM_YAW_RIGHT,
	ANTIAIM_YAW_SIDE,
	ANTIAIM_YAW_FAKE_LBY1,
	ANTIAIM_YAW_FAKE_LBY2,
	ANTIAIM_YAW_JITTER,
	ANTIAIM_YAW_BACKJITTER,
	ANTIAIM_YAW_FAKE_SIDE_LBY,
	ANTIAIM_YAW_RANDOM
};

static char* AntiaimYawNames[] =
{
	"None",
	"Spin",
	"Static Forward",
	"Static Right",
	"Static Backwards",
	"Static Left",
	"Backwards",
	"Left",
	"Right",
	"Side",
	"Fake LBY 1",
	"Fake LBY 2",
	"Jitter",
	"Backjitter",
	"Fake Side LBY",
	"Random"
};

enum AntiAimPitch_t
{
	ANTIAIM_PITCH_NONE,

	ANTIAIM_PITCH_DOWN,
	ANTIAIM_PITCH_UP,
	ANTIAIM_PITCH_DANCE,
	ANTIAIM_PITCH_FAKEUP,
	ANTIAIM_PITCH_FAKEDOWN,
	ANTIAIM_PITCH_RANDOM
};

static char* AntiaimPitchNames[] =
{
	"None",
	"Down",
	"Up",
	"Dance",
	"Fake Up",
	"Fake Down",
	"Random"
};

enum AntiAimThirdperson_t
{
	ANTIAIM_THIRDPERSON_REAL,
	ANTIAIM_THIRDPERSON_FAKE,
	ANTIAIM_THIRDPERSON_BOTH
};

static char* AntiaimThirdpersonAngle[] =
{
	"Real",
	"Fake",
	"All"
};

enum ChamsType_t
{
	CHAM_TYPE_MATERIAL,
	CHAM_TYPE_FLAT,
	CHAM_TYPE_VAPOR,
	CHAM_TYPE_GLOW
};

enum ak47skins {
	DEFAULT_AK47SKIN,
	First_Class,
	Red_Laminate,
	Case_Hardened,
	Safari_Mesh,
	Jungle_Spray,
	Predator,
	Black_Laminate,
	Fire_Serpent,
	Frontside_Misty,
	Cartel,
	Emerald_Pinstripe,
	Point_Disarray,
	Blue_Laminate,
	Redline,
	Vulcan,
	Jaguar,
	Jet_Set,
	Fuel_Injector,
	Wasteland_Rebel,
	Elite_Build,
	Hydroponic,
	Aquamarine_Revenge,
	Neon_Revolution,
	BloodsportAK,
	Orbit_Mk01,
	The_Empress,
};

enum galilskins {
	Default_GALILSKIN,
	Orange_DDPAT,
	Eco,
	Winter_Forest,
	Sage_Spray,
	VariCamo,
	Chatterbox,
	Stone_Cold,
	Shattered,
	Kami,
	Blue_Titanium,
	Urban_Rubble,
	Hunting_Blind,
	Sandstorm,
	Tuxedo,
	Cerberus,
	Aqua_Terrace,
	Rocket_Pop,
	Firefight,
	Black_Sand,
	Crimson_Tsunami,
	sugar_rush,
};

enum m4a4skins {
	DEFAULT_M4A4,
	Desert_Storm,
	Tornadom4a4,
	Radiation_Hazard,
	Jungle_Tiger,
	Modern_Hunter,
	Urban_DDPAT,
	Bullet_Rain,
	Faded_Zebra,
	Zirka,
	Asiimov,
	Howl,
	X_Ray,
	Desert_Strike,
	Desolate_Space,
	Griffin,
	Dragon_King,
	Poseidon,
	Daybreak,
	Evil_Daimyo,
	Royal_Paladin,
	The_Battlestar,
	Buzz_Kill,
	Hell_Fire,
};

enum m4a1skins {
	DEFAULT_M4A1,
	Dark_Water,
	Hyper_Beast,
	Boreal_Forest,
	VariCamoM4,
	Golden_Coil,
	Nitro,
	Bright_Water,
	Mecha_Industries,
	Atomic_Alloy,
	Blood_Tiger,
	Guardian,
	Master_Piece,
	Knight,
	Cyrex,
	Basilisk,
	Icarus_Fell,
	Hot_Rodm4a1,
	Chanticos_Fire,
	Flashback,
	Decimator,
	Briefing,
	Leaded_Glass,
};

enum famasskins {
	DEFAULT_FAMAS,
	Contrast_Spray,
	Colony,
	Cyanospatter,
	Djinn,
	Afterimage,
	Doomkitty,
	Survivor_Z,
	Spitfire,
	Teardown,
	Hexane,
	PulseFamas,
	Sergeant,
	Styx,
	Valence,
	Neural_Net,
	Roll_Cage,
	Mecha_IndustriesFamas,
	Macabre,
};

enum sg553skins {
	DEFAULTSG553,
	Tornado,
	Anodized_Navy,
	Bulldozer,
	Ultraviolet,
	Waves_Perforated,
	Wave_Spray,
	Gator_Mesh,
	Damascus_Steel,
	Pulse,
	Army_Sheen,
	Traveler,
	Fallout_Warning,
	Tiger_Moth,
	Cyrexsg553,
	Atlas,
	Aerial,
	Triarch,
	Phantom,
};


enum augskins {
	DEFAULTAUG,
	Wings,
	Copperhead,
	Bengal_Tiger,
	Condemned,
	Hot_Rod,
	Storm,
	Contractor,
	ColonyAug,
	Aristocrat,
	Anodized_NavyAug,
	Ricochet,
	Chameleon,
	TorqueAug,
	Radiation_HazardAug,
	Daedalus,
	Akihabara_Accept,
	Fleet_Flock,
	Syd_Mead,
	Triqua,
};

enum glockskins {
	DEFAULT_GLOCKSKIN,
	Groundwater,
	Candy_Apple,
	Fade,
	Night,
	Dragon_Tattoo,
	Twilight_Galaxy,
	Brass,
	Catacombs,
	Wraiths,
	Wasteland_RebelGlock,
	Sand_DuneGlock,
	Steel_Disruption,
	Blue_Fissure,
	Death_Rattle,
	Water_Elemental,
	Reactor,
	Grinder,
	Bunsen_Burner,
	Royal_Legion,
	WeaselGlock,
	Ironwork,
	Off_World,
};

enum revolverskins {
	DEFAULT_REVOLVERSKIN,
	Crimson_Web,
	Bone_Mask,
	Urban_Perforated,
	Waves_Perforated2,
	Orange_Peel,
	Urban_Masked,
	Jungle_Dashed,
	Sand_Dashed,
	Urban_Dashed,
	Dry_Season,
	FadeR8,
	Amber_Fade,
	Reboot,
	Llama_Cannon,
};

enum fivesevenskins {
	DEFAULTFIVESEVEN,
	Candy_Apple57,
	Case_Hardened57,
	Contractor57,
	Forest_Night,
	Orange_Peel57,
	Jungle,
	Anodized_Gunmetal,
	Nightshade,
	Silver_Quartz,
	Nitro57,
	Kami57,
	Copper_Galaxy,
	Fowl_Play,
	Hot_Shot,
	Urban_Hazard,
	Monkey_Buisness,
	Neon_Kimono,
	Orange_Kimono57,
	Crimson_Kimono,
	Retrobution,
	Trimuvirate,
	Violent_Daimyo,
	Scumbria57,
	Capillary,
	Hyper_Beast57,
};

enum awpskins {
	DEFAULTAWP,
	BOOM,
	Dragon_Lore,
	Pink_DDPAT,
	Elite_BuildAWP,
	Snake_Camo,
	Lightning_Strike,
	Safari_MeshAWP,
	Corticera,
	RedlineAWP,
	Man_o_war,
	Phobos,
	Graphite,
	Electric_Hive,
	Pit_Viper,
	AsiimovAWP,
	Worm_God,
	Medusa,
	Sun_in_Leo,
	Hyper_BeastAWP,
	Fever_Dream,
	Oni_Taiji,
};

enum ssg08skins {
	DEFAULTSSG,
	Lichen_Dashed,
	Dark_Waterssg,
	Blue_Spruce,
	Sand_Dune,
	Mayan_Dreams,
	Blood_in_the_Water,
	Big_Iron,
	Tropical_Storm,
	Acid_Fade,
	Slashed,
	Detour,
	Necropos,
	Abyss,
	Ghost_Crusader,
	Dragonfire,
	Deaths_Head,
};

enum g3sg1skins {
	DEFAULTG3,
	Desert_Stormg3,
	Arctic_Camo,
	Contractorg3,
	Safari_Meshg3,
	Polar_Camo,
	Jungle_Dashedg3,
	VariCamog3,
	Flux,
	Demeter,
	Azure_Zebra,
	Green_Apple,
	Orange_Kimonog3,
	Murky,
	Chronos,
	The_Executioner,
	Orange_Crash,
	Ventilator,
	Stingerg3s,
	Hunterg3,
};

enum scar20skins {
	DEFAULTSCAR,
	Splash_Jam,
	StormScar,
	ContractorScar,
	Carbon_Fiber,
	Sand_Mesh,
	Palm,
	Emerald,
	Green_Marine,
	Crimson_WebScar,
	Cardiac,
	Army_SheenScar,
	CyrexScar,
	Grotto,
	Outbreak,
	Bloodsport,
	Powercore,
	Blueprint,
	Jungle_Slipstream,
};

enum negevskins {
	DEFAULTNEGEV,
	Anodized_NavyNegev,
	Man_o_warNegev,
	PalmNegev,
	CaliCamo,
	TerrainNegev,
	Army_SheenNegev,
	Bratatat,
	Desert_StrikeNegev,
	Nuclear_Waste,
	Loudmouth,
	Power_Loader,
	Dazzle,
};

enum m249skins {
	DEFAULTM249,
	Contrast_Spraym249,
	Blizzard_Marbleized,
	Nebula_Crusader,
	Jungle_DDPAT,
	Gator_Meshm249,
	Magma,
	System_Lock,
	Shipping_Forecast,
	Impact_Drill,
	Spectre,
	Emerald_Poison_Dart,
};

enum novaskins {
	DEFAULTNOVA,
	Candy_AppleNova,
	Blaze_OrangeNova,
	Modern_HunterNova,
	Forest_LeavesNova,
	Bloomstick,
	Sand_DuneNova,
	Polar_MeshNova,
	WalnutNova,
	PredatorNova,
	Tempest,
	GraphiteNova,
	Ghost_Camo,
	Rising_Skull,
	Antique,
	Green_AppleNova,
	Caged_Steel,
	Koi,
	Moon_in_Libra,
	RangerNova,
	Hyper_BeastNova,
	Exo,
	Gila,
};

enum tec9skins {
	DEFAULT_TEC9,
	GroundwaterTEC9,
	AvalancheTEC9,
	Terrace,
	Urban_DDPATTEC9,
	Ossified,
	Hades,
	Brasstec,
	VariCamoTEC9,
	Nuclear_Threat,
	Red_Quartz,
	TornadoTEC9,
	Blue_TitaniumTEC9,
	Army_MeshTEC9,
	Titanium_Bit,
	SandstormTEC9,
	IsaacTEC9,
	Jambiya,
	Toxic,
	Bamboo_Forest,
	Re_Entry,
	Ice_Cap,
	Fuel_Injectortec,
	Cut_Out,
	Cracked_Opal,
};

enum deagleskins {
	DEFAULTDEAGLE,
	Blaze,
	Pilotdeagle,
	Midnight_Storm,
	Urban_DDPATdeagle,
	Nightdeagle,
	Hypnotic,
	Mudderdeagle,
	Golden_Koi,
	Cobalt_Disruption,
	Crimson_Webdeagle,
	Urban_Rubbledeagle,
	Naga,
	Hand_Cannon,
	Heirloom,
	Meteorite,
	Kumicho_Dragon,
	Conspiracy,
	Corinthian,
	Bronze_Decodeagle,
	Sunset_Storm,
	Directive,
	Oxide_Blaze,
};

enum cz75skins {
	DEFAULTCZ75,
	Pole_Position,
	Crimson_Webcz,
	Hexanecz,
	Tread_Plate,
	The_Fuschia_Is_Now,
	Victoria,
	Tuxedocz,
	Army_Sheencz,
	Poison_Dart,
	Nitrocz,
	Chalicecz,
	Twist,
	Tigriscz,
	Green_Plaidcz,
	Emeraldcz,
	Yellow_Jacket,
	Red_Astor,
	Imprintcz,
	Xiangliu,
	Tacticat,
	Polymercz,
};

enum uspskins {
	DEFAULTUSP,
	Forest_Leaves,
	Dark_WaterUSP,
	Overgrowth,
	Caiman,
	Blood_TigerUSP,
	Serum,
	Kill_Confirmed,
	Night_Ops,
	Stainless,
	GuardianUSP,
	Orion,
	Road_Rash,
	Royal_Blue,
	Business_Class,
	Para_Green,
	Torque,
	Lead_Conduit,
	CyrexUSP,
	Neo_NoirUSP,
	BlueprintUSP
};

enum p2000skins {
	Defaultp2000,
	Silverp2000,
	Grassland_Leavesp2000,
	Granite_Marbleized,
	Handgun,
	Scorpionp2000,
	Grasslandp2000,
	Corticerap2000,
	Ocean_Foam,
	Pulsep2000,
	Amber_Fadep2000,
	Red_FragCam,
	Chainmailp2000,
	Coach_Class,
	Ivory,
	Fire_Elemental,
	Pathfinder,
	Imperial,
	Oceanic,
	Imperial_Dragon,
	Turf,
	Woodsman,
};

enum dualiesskins {
	DEFAULTDUALIES,
	Anodized_Navyelites,
	Stainedelites,
	Contractorelites,
	Colonyelites,
	Demolition,
	Dualing_Dragons,
	Black_Limba,
	Cobalt_Quartzelites,
	Hemoglobin,
	Urban_Shock,
	Marina,
	Panther,
	Retribution,
	Briar,
	Duelist,
	Moon_in_Libraelites,
	Cartelelites,
	Ventilators,
	Royal_Consorts,
	Cobra_Strike,
};

enum p250skins {
	DEFAULTP250,
	Metallic_DDPATp250,
	Whiteoutp250,
	Splashp250,
	Gunsmokep250,
	Modern_Hunterp250,
	Bone_Maskp250,
	Boreal_Forestp250,
	Sand_Dunep250,
	Nuclear_Threatp250,
	Mehndip250,
	Facetsp250,
	Hivep250,
	Wingshotp250,
	Muertosp250,
	Steel_Disruptionp250,
	Undertowp250,
	Franklinp250,
	Supernovap250,
	Contaminationp250,
	Cartelp250,
	Valencep250,
	Crimson_Kimonop250,
	Mint_Kimonop250,
	Asiimovp250,
	Iron_Cladp250,
	See_Ya_Later,
	Red_Rock,
	Ripple,
};

enum sawedoffskins {
	DEFAULTSAWEDOFF,
	FirstClass,
	ForestDDPAT,
	SnakeCamo,
	OrangeDDPAT,
	Copper,
	Origami,
	SageSpray,
	IrradiatedAlert,
	Mosaico,
	Serenity,
	AmberFade,
	FullStop,
	Highwayman,
	TheKraken,
	RustCoat,
	BambooShadow,
	Yorick,
	Fubar,
	Limelight,
	Wasteland_Princess,
	Zander,
	Morris,
};

enum xm1014skins {
	DEFAULTXM,
	Teclu_Burner,
	Blaze_Orange,
	VariCamo_Blue,
	Blue_Steel,
	Blue_SpruceXM,
	Grassland,
	Urban_PerforatedXM,
	Fallout_WarningXM,
	JungleXM,
	Scumbria,
	CaliCamoXM,
	Tranquility,
	Red_Python,
	Heaven_Guard,
	Red_Leather,
	Bone_Machine,
	Quicksilver,
	Black_Tie,
	Slipstream,
	Seasons,
	Ziggy,
};

enum mag7skins {
	DEFAULTMAG7,
	Counter_Terracemag7,
	Metallic_DDPATmag7,
	Silvermag7,
	Stormmag7,
	Bulldozermag7,
	Heatmag7,
	Sand_Dunemag7,
	Irradiated_Alertmag7,
	Mementomag7,
	Hazardmag7,
	Cobalt_Coremag7,
	Heaven_Guardmag7,
	Praetorianmag7,
	Firestartermag7,
	Seabirdmag7,
	Petroglyph,
	Sonar,
	Hard_Water,
};

enum mac10skins {
	DEFAULTMAC10,
	Tornadomac10,
	Candy_Applemac10,
	Silvermac10,
	Urban_DDPATmac10,
	Fademac10,
	Neon_Ridermac10,
	Ultravioletmac10,
	Palmmac10,
	Gravenmac10,
	Tattermac10,
	Carnivoremac10,
	Amber_Fademac10,
	Rangeenmac10,
	Heatmac10,
	Cursemac10,
	Indigomac10,
	Commutermac10,
	Lapis_Gatormac10,
	Nuclear_Gardenmac10,
	Malachitemac10,
	Last_Dive,
	Aloha,
	Oceanicmac10,
};

enum mp9skins {
	DEFAULTMP9,
	Ruby_Poison_Dartmp9,
	Hot_Rodmp9,
	Stormmp9,
	Bulldozermp9,
	Hypnoticmp9,
	Sand_Dashedmp9,
	Orange_Peelmp9,
	Dry_Seasonmp9,
	Dark_Agemp9,
	Rose_Ironmp9,
	Green_Plaidmp9,
	Setting_Sunmp9,
	Dartmp9,
	Deadly_Poisonmp9,
	Pandoras_Boxmp9,
	Bioleakmp9,
	Airlock,
	Sand_Scale,
	Goo,
};

enum mp7skins {
	DEFAULTMP7,
	Whiteoutmp7,
	Forest_DDPATmp7,
	Anodized_Navymp7,
	Skullsmp7,
	Gunsmokemp7,
	Orange_Peelmp7,
	Army_Reconmp7,
	Groundwatermp7,
	Ocean_Foammp7,
	Special_Deliverymp7,
	Full_Stopmp7,
	Urban_Hazardmp7,
	Olive_Plaidmp7,
	Armor_Coremp7,
	Asterionmp7,
	Nemesismp7,
	Impiremp7,
	Cirrus,
	Akoben,
};

enum umpskins {
	DEFAULTUMP,
	Blazeump,
	Gunsmokeump,
	Urban_DDPATump,
	Carbon_Fiberump,
	Grand_Prixump,
	Caramelump,
	Fallout_Warningump,
	Scorchedump,
	Bone_Pileump,
	Delusionump,
	Corporalump,
	Indigoump,
	Labyrinthump,
	Minotaurs_Labyrinthump,
	Riotump,
	Primal_Saberump,
	BriefingUMP,
	Scaffold,
	Metal_Flowers,
	Exposure,
};

enum p90skins {
	DEFAULTP99,
	Leatherp90,
	Virusp90,
	Stormp90,
	Cold_Bloodedp90,
	Glacier_Meshp90,
	Sand_Sprayp90,
	Death_by_Kittyp90,
	Ash_Woodp90,
	Fallout_Warningp90,
	Scorchedp90,
	Emerald_Dragonp90,
	Teardownp90,
	Blind_Spotp90,
	Trigonp90,
	Desert_Warfarep90,
	Modulep90,
	Asiimovp90,
	Shapewoodp90,
	Elite_Buildp90,
	Chopperp90,
	Grim,
	Shallow_Grave,
};

enum bizonskins {
	DEFAULTPPBIZON,
	Photic_Zonebizon,
	Blue_Streakbizon,
	Modern_Hunterbizon,
	Forest_Leavesbizon,
	Carbon_Fiberbizon,
	Sand_Dashedbizon,
	Urban_Dashedbizon,
	Brassbizon,
	Irradiated_Alertbizon,
	Rust_Coatbizon,
	Water_Sigilbizon,
	Night_Opsbizon,
	Cobalt_Halftonebizon,
	Harvesterbizon,
	Antiquebizon,
	Osirisbizon,
	Chemical_Greenbizon,
	Fuel_Rodbizon,
	Bamboo_Printbizon,
	Judgement_of_Anubisbizon,
	Jungle_Slipstreambizon,
	High_Roller,
};

enum knifeskins {
	DEFAULTKNIFE,
	Safari_MeshKnife,
	Boreal_ForestKnife,
	Doppler_Phase_1Knife,
	Doppler_Phase_2Knife,
	Doppler_Phase_3Knife,
	Doppler_Phase_4Knife,
	RubyKnife,
	SapphireKnife,
	Black_PearlKnife,
	SlaughterKnife,
	FadeKnife,
	Crimson_WebKnife,
	NightKnife,
	Blue_SteelKnife,
	StainedKnife,
	Case_HardenedKnife,
	UltravioletKnife,
	Urban_MaskedKnife,
	Damascus_SteelKnife,
	ScorchedKnife,
	Bright_WaterKnife,
	EmeraldKnife,
	Gamma_Doppler_Phase_1Knife,
	Gamma_Doppler_Phase_2Knife,
	Gamma_Doppler_Phase_3Knife,
	Gamma_Doppler_Phase_4Knife,
	FreehandKnife,
	Tiger_ToothKnife,
	Rust_CoatKnife,
	Marble_FadeKnife,
	AutotronicKnife,
	Black_LaminateKnife,
	LoreKnife,
	Forest_DDPATKnife,
};

enum playermodel {
	No_Model,
	Reina_Kousaka,
	Mirai_Nikki,
	Banana_Joe,
};

enum knifemodel {
	No_Knife_Model,
	Minecraft_Pickaxe,
	Anime_Model,
	Banana
};

enum hitsounds
{
	NOHITSOUND,
	CODSOUND,
	Anime,
	Bubbles,
	Custom,
};

static char* HitSounds[] =
{
	"None",
	"COD",
	"Anime",
	"Bubbles",
	"Custom",
};

enum aimspotslegit
{
	Head,
	Neck,
	Chest,
	Stomach,
};

static char* AimSpots[] =
{
	"Head", // 8
	"Neck", // 7
	"Chest", // 6 
	"Stomach", // 4
};

enum triggerbot_spots
{
	tbHead,
	tbChest,
	tbStomach,
	tbAll,
};

static char* TriggerSpots[] =
{
	"Head", // 8
	"Chest", // 7
	"Stomach", // 6 
	"All", // 4
};

enum AutoStrafer_t
{
	AUTOSTRAFER_NONE,
	AUTOSTRAFER_LEGIT,
	AUTOSTRAFER_RAGE
};

enum Knives_t
{
	KNIFE_DEFAULT,
	KNIFE_BAYONET,
	KNIFE_FLIP,
	KNIFE_GUT,
	KNIFE_KARAMBIT,
	KNIFE_M9BAYONET,
	KNIFE_HUNTSMAN,
	KNIFE_FALCHION,
	KNIFE_BOWIE,
	KNIFE_BUTTERFLY,
	KNIFE_PUSHDAGGER
};

enum Gloves_t
{
	GLOVE_DEFAULT,
	GLOVE_BLOODHOUND,
	GLOVE_SPORT,
	GLOVE_SLICK,
	GLOVE_LEATHERWRAP,
	GLOVE_MOTO,
	GLOVE_SPECIALIST
};


enum GlotStyle_t
{
	GLOWTYPE_OUTLINE,
	GLOWTYPE_FULLBLOOM,
	GLOWTYPE_AURA,
	GLOWTYPE_TEXTURE,
	GLOWTYPE_TEXTURE_PULSATING
};



enum keybinds_t {
	DISABLED_KEY,
	MOUSE_1,
	MOUSE_2,
	MOUSE_3,
	MOUSE_4,
	MOUSE_5,
	ALT,
	CTRL,
	SHIFT,
	CAPS,
	TAB,
};

static char* ak47SkinNames[] = {
	"Default",
	"First Class",
	"Red Laminate",
	"Case Hardened",
	"Safari Mesh",
	"Jungle Spray",
	"Predator",
	"Black Laminate",
	"Fire Serpent",
	"Frontside Misty",
	"Cartel",
	"Emerald Pinstripe",
	"Point Disarray",
	"Blue Laminate",
	"Redline",
	"Vulcan",
	"Jaguar",
	"Jet Set",
	"Fuel Injector",
	"Wasteland Rebel",
	"Elite Build",
	"Hydroponic",
	"Aquamarine Revenge",
	"Neon Revolution",
	"Bloodspor",
	"Orbit Mk01",
	"The Empress",
};

static char* galilSkinNames[] = {
	"Default",
	"Orange DDPAT",
	"Eco",
	"Winter Forest",
	"Sage Spray",
	"VariCamo",
	"Chatterbox",
	"Stone Cold",
	"Shattered",
	"Kami",
	"Blue Titanium",
	"Urban Rubble",
	"Hunting Blind",
	"Sandstorm",
	"Tuxedo",
	"Cerberus",
	"Aqua Terrace",
	"Rocket Pop",
	"Firefight",
	"Black Sand",
	"Crimson Tsunami",
	"Sugar Rush",
};

static char* m4a4Skinnames[] = {
	"Default",
	"Desert Storm",
	"Tornado",
	"Radiation Hazard",
	"Jungle Tiger",
	"Modern Hunter",
	"Urban DDPAT",
	"Bullet Rain",
	"Faded Zebra",
	"Zirka",
	"Asiimov",
	"Howl",
	"X - Ray",
	"Desert - Strike",
	"Desolate Space",
	"Griffin",
	"Dragon King",
	"Poseidon",
	"Daybreak",
	"Evil Daimyo",
	"Royal Paladin",
	"The Battlestar",
	"Buzz Kill",
	"Hellfire",
};
static char* m4a1Skinnames[] = {
	"Default",
	"Dark Water",
	"Hyper Beast",
	"Boreal Forest",
	"VariCamo",
	"Golden Coil",
	"Nitro",
	"Bright Water",
	"Mecha Industries",
	"Atomic Alloy",
	"Blood Tiger",
	"Guardian",
	"Master Piece",
	"Knight",
	"Cyrex",
	"Basilisk",
	"Icarus Fell",
	"Hot Rod",
	"Chanticos Fire",
	"Flashback",
	"Decimator",
	"Briefing",
	"Leaded Glass",
};

static char* famasSkinNames[]{
	"Default",
	"Contrast Spray",
	"Colony",
	"Cyanospatte",
	"Djinn",
	"Afterimage",
	"Doomkitty",
	"Survivor Z",
	"Spitfire",
	"Teardown",
	"Hexane",
	"Pulse",
	"Sergeant",
	"Styx",
	"Valence",
	"Neural Net",
	"Roll Cage",
	"Mecha Industries",
	"Macabre",
};

static char* sg553SkinNames[]{
	"Default",
	"Tornado",
	"Anodized Navy",
	"Bulldozer",
	"Ultraviolet",
	"Waves Perforated",
	"Wave Spray",
	"Gator Mesh",
	"Damascus Steel",
	"Pulse",
	"Army Sheen",
	"Traveler",
	"Fallout Warning",
	"Tiger Moth",
	"Cyrex",
	"Atlas",
	"Aerial",
	"Triarch",
	"Phantom",
};

static char* augSkinNames[]{
	"Default",
	"Wings",
	"Copperhead",
	"Bengal Tiger",
	"Condemned",
	"Hot Rod",
	"Storm",
	"Contractor",
	"Colony",
	"Aristocrat",
	"Anodized Navy",
	"Ricochet",
	"Chameleon",
	"Torque",
	"Radiation Hazard",
	"Daedalus",
	"Akihabara Accept",
	"Fleet Flock",
	"Syd Mead",
	"Triqua",
};

static char* glockSkinNames[]{
	"Default",
	"Groundwater",
	"Candy Apple",
	"Fade",
	"Night",
	"Dragon Tattoo",
	"Twilight Galaxy",
	"Brass",
	"Catacombs",
	"Wraiths",
	"Wasteland Rebel",
	"Sand Dune",
	"Steel Disruption",
	"Blue Fissure",
	"Death Rattle",
	"Water Elemental",
	"Reactor",
	"Grinder",
	"Bunsen Burner",
	"Royal Legion",
	"Weasel",
	"Ironwork",
	"Off World",
};


static char* revolverSkinNames[]{
	"Default",
	"Crimson Web",
	"Bone Mask",
	"Urban Perforated",
	"Waves Perforated",
	"Orange Peel",
	"Urban Masked",
	"Jungle Dashed",
	"Sand Dashed",
	"Urban Dashed",
	"Dry Season",
	"Fade",
	"Amber Fade",
	"Reboot",
	"Llama Cannon",
};

static char* fivesevenSkinNames[]{
	"Default",
	"Candy Apple",
	"Case Hardened",
	"Contractor",
	"Forest Night",
	"Orange Peel",
	"Jungle",
	"Anodized Gunmetal",
	"Nightshade",
	"Silver Quartz",
	"Nitro",
	"Kami",
	"Copper Galaxy",
	"Fowl Play",
	"Hot Shot",
	"Urban Hazard",
	"Monkey Buisness",
	"Neon Kimono",
	"Orange Kimono",
	"Crimson Kimono",
	"Retrobution",
	"Violent Daimyo",
	"Scumbria",
	"Capillary",
	"Hyper Beast",
};

static char* awpSkinNames[]{
	"Default",
	"BOOM",
	"Dragon Lore",
	"Pink DDPAT",
	"Elite Build",
	"Snake Camo",
	"Lightning Strike",
	"Safari Mesh",
	"Corticera",
	"Redline",
	"Man-o-war",
	"Phobos",
	"Graphite",
	"Electric Hive",
	"Pit Viper",
	"Asiimov",
	"Worm God",
	"Medusa",
	"Sun in Leo",
	"Hyper Beast",
	"Fever Dream",
	"Oni Taiji",
};

static char* ssg08SkinNames[]{
	"Default",
	"Lichen Dashed",
	"Dark Water",
	"Blue Spruce",
	"Sand Dune",
	"Mayan Dreams",
	"Blood in the Water",
	"Big Iron",
	"Tropical Storm",
	"Acid Fade",
	"Slashed",
	"Detour",
	"Necropos",
	"Abyss",
	"Ghost Crusader",
	"Dragonfire",
	"Deaths Head",
};

static char* g3sg1SkinNames[]{
	"Default",
	"Desert Storm",
	"Arctic Camo",
	"Contractor",
	"Safari Mesh",
	"Polar Camo",
	"Jungle Dashed",
	"VariCamo",
	"Flux",
	"Demeter",
	"Azure Zebra",
	"Green Apple",
	"Orange Kimono",
	"Murky",
	"Chronos",
	"The Executioner",
	"Orange Crash",
	"Ventilator,"
	"Stinger,"
	"Hunter,"
};

static char* scar20Names[]{
	"Default",
	"Splash Jam",
	"Storm",
	"Contractor",
	"Carbon Fiber",
	"Sand Mesh",
	"Palm",
	"Emerald",
	"Green Marine",
	"Crimson Web",
	"Cardiac",
	"Army Sheen",
	"Cyrex",
	"Grotto",
	"Outbreak",
	"BloodSport",
	"Powercore",
	"Blueprint",
	"Jungle Slipstream",
};

static char* negevNames[]{
	"Default",
	"Anodized Navy",
	"Man-o-war",
	"Palm",
	"CaliCamo",
	"Terrain",
	"Army Sheen",
	"Bratatat",
	"Desert - Strike",
	"Nuclear Waste",
	"Loudmouth",
	"Power Loader",
	"Dazzle",
};

static char* m249Names[]{
	"Default",
	"Contrast Spray",
	"Blizzard Marbleized",
	"Nebula Crusader",
	"Jungle DDPAT",
	"Gator Mesh",
	"Magma",
	"System Lock",
	"Shipping Forecast",
	"Impact Drill",
	"Spectre",
	"Emerald Poison Dart",
};

static char* novaNames[]{
	"Default",
	"Candy Apple",
	"Blaze Orange",
	"Modern Hunter",
	"Forest Leaves",
	"Bloomstick",
	"Sand Dune",
	"Polar Mesh",
	"Walnut",
	"Predator",
	"Tempest",
	"Graphite",
	"Ghost Camo",
	"Rising Skull",
	"Antique",
	"Green Apple",
	"Caged Steel",
	"Koi",
	"Moon in Libra",
	"Ranger",
	"Hyper Beast",
	"Exo",
	"Gila",
};

static char* tec9SkinNames[]{
	"Default",
	"Groundwater",
	"Avalanche",
	"Terrace",
	"Urban DDPAT",
	"Ossified",
	"Hades",
	"Brass",
	"VariCamo",
	"Nuclear Threat",
	"Red Quartz",
	"Tornado",
	"Blue Titanium",
	"Army Mesh",
	"Titanium Bit",
	"Sandstorm",
	"Isaac",
	"Jambiya",
	"Toxic",
	"Bamboo Forest",
	"Re-Entry",
	"Ice Cap",
	"Fuel Injector",
	"Cut Out",
	"Cracked Opal",
};

static char* deagleSkinNames[]{
	"Deafult",
	"Blaze",
	"Pilot",
	"Midnight Storm",
	"Urban DDPAT",
	"Night",
	"Hypnotic",
	"Mudder",
	"Golden Koi",
	"Cobalt Disruption",
	"Crimson Web",
	"Urban Rubble",
	"Naga",
	"Hand Cannon",
	"Heirloom",
	"Meteorite",
	"Kumicho Dragon",
	"Conspiracy",
	"Corinthian",
	"Bronze Deco",
	"Sunset Storm",
	"Directive",
	"Oxide Blaze",
};

static char* cz75SkinNames[]{
	"Deafult",
	"Pole Position",
	"Crimson Web",
	"Hexane",
	"Tread Plate",
	"The Fuschia Is Now",
	"Victoria",
	"Tuxedo",
	"Army Sheen",
	"Poison Dart",
	"Nitro",
	"Chalice",
	"Twist",
	"Tigris",
	"Green Plaid",
	"Emerald",
	"Yellow Jacket",
	"Red Astor",
	"Imprint",
	"Xiangliu",
	"Tacticat",
	"Polymer",
};

static char* uspSkinNames[]{
	"Default",
	"Forest Leaves",
	"Dark Water",
	"Overgrowth",
	"Caiman",
	"Blood Tiger",
	"Serum",
	"Kill Confirmed",
	"Night Ops",
	"Stainless",
	"Guardian",
	"Orion",
	"Road Rash",
	"Royal Blue",
	"Business Class",
	"Para Green",
	"Torque",
	"Lead Conduit",
	"Cyrex",
	"Neo-Noir",
	"Blueprint",
};

static char* p2000SkinNames[]{
	"Default",
	"Silver",
	"Grassland Leaves",
	"Granite Marbleized",
	"Handgun",
	"Scorpion",
	"Grassland",
	"Corticera",
	"Ocean Foam",
	"Pulse",
	"Amber Fade",
	"Red FragCam",
	"Chainmail",
	"Coach Class",
	"Ivory",
	"Fire Elemental",
	"Pathfinder",
	"Imperial",
	"Oceanic",
	"Imperial Dragon",
	"Turf",
	"Woodsman",
};

static char* dualiesSkinNames[]{
	"Default",
	"Anodized Navy",
	"Stained",
	"Contractor",
	"Colony",
	"Demolition",
	"Dualing Dragons",
	"Black Limba",
	"Cobalt Quartz",
	"Hemoglobin",
	"Urban Shock",
	"Marina",
	"Panther",
	"Retribution",
	"Briar",
	"Duelist",
	"Moon in Libra",
	"Cartel",
	"Ventilators",
	"Royal Consorts",
	"Cobra Strike",
};

static char* p250SkinNames[]{
	"Default",
	"Metallic DDPAT",
	"Whiteout",
	"Splash",
	"Gunsmoke",
	"Modern Hunter",
	"Bone Mask",
	"Boreal Forest",
	"Sand Dune",
	"Nuclear Threat",
	"Mehndi",
	"Facets",
	"Hive",
	"Wingshot",
	"Muertos",
	"Steel Disruption",
	"Undertow",
	"Franklin",
	"Supernova",
	"Contamination",
	"Cartel",
	"Valence",
	"Crimson Kimono",
	"Mint Kimono",
	"Asiimov",
	"Iron Clad",
	"See Ya Later",
	"Red Rock",
	"Ripple",
};

static char* sawedoffSkinNames[]{
	"Default",
	"First Class",
	"Forest DDPAT",
	"Snake Camo",
	"Orange DDPAT",
	"Copper",
	"Origami",
	"Sage Spray",
	"Irradiated Alert",
	"Mosaico",
	"Serenity",
	"Amber Fade",
	"Full Stop",
	"Highwayman",
	"The Kraken",
	"Rust Coat",
	"Bamboo Shadow",
	"Yorick",
	"Fubar",
	"Limelight",
	"Wasteland Princess",
	"Zander",
	"Morris",
};

static char* xm1014SkinNames[]{
	"Default",
	"Teclu Burner",
	"Blaze Orange",
	"VariCamo Blue",
	"Blue Steel",
	"Blue Spruce",
	"Grassland",
	"Urban Perforated",
	"Fallout Warning",
	"Jungle",
	"Scumbria",
	"CaliCamo",
	"Tranquility",
	"Red Python",
	"Heaven Guard",
	"Red Leather",
	"Bone Machine",
	"Quicksilver",
	"Black Tie",
	"Slipstream",
	"Seasons",
	"Ziggy",
};

static char* mag7SkinNames[]{
	"Default",
	"Counter Terrace",
	"Metallic DDPAT",
	"Silver",
	"Storm",
	"Bulldozer",
	"Heat",
	"Sand Dune",
	"Irradiated Alert",
	"Memento",
	"Hazard",
	"Cobalt Core",
	"Heaven Guard",
	"Praetorian",
	"Firestarter",
	"Seabird",
	"Petroglyph",
	"Sonar",
	"Hard Water",
};

static char* mac10SkinNames[]{
	"Default",
	"Tornado",
	"Candy Apple",
	"Silver",
	"Urban DDPAT",
	"Fade",
	"Neon Rider",
	"Ultraviolet",
	"Palm",
	"Graven",
	"Tatter",
	"Carnivore",
	"Amber Fade",
	"Rangeen",
	"Heat",
	"Curse",
	"Indigo",
	"Commuter",
	"Lapis Gator",
	"Nuclear Garden",
	"Malachite",
	"Last Dive",
	"Aloha",
	"Oceanic",
};

static char* mp9SkinNames[]{
	"Default",
	"Ruby Poison Dart",
	"Hot Rod",
	"Storm",
	"Bulldozer",
	"Hypnotic",
	"Sand Dashed",
	"Orange Peel",
	"Dry Season",
	"Dark Age",
	"Rose Iron",
	"Green Plaid",
	"Setting Sun",
	"Dart",
	"Deadly Poison",
	"Pandoras Box",
	"Bioleak",
	"Airlock",
	"Sand Scale",
	"Goo",
};

static char* mp7SkinNames[]{
	"Default",
	"Whiteout",
	"Forest DDPAT",
	"Anodized Navy",
	"Skulls",
	"Gunsmoke",
	"Orange Peel",
	"Army Recon",
	"Groundwater",
	"Ocean Foam",
	"Special Delivery",
	"Full Stop",
	"Urban Hazard",
	"Olive Plaid",
	"Armor Core",
	"Asterion",
	"Nemesis",
	"Impire",
	"Cirrus",
	"Akoben",
};

static char* umpSkinNames[]{
	"Default",
	"Blaze",
	"Gunsmoke",
	"Urban DDPAT",
	"Carbon Fiber",
	"Grand Prix",
	"Caramel",
	"Fallout Warning",
	"Scorched",
	"Bone Pile",
	"Delusion",
	"Corporal",
	"Indigo",
	"Labyrinth",
	"Minotaurs Labyrinth",
	"Riot",
	"Primal Saber",
	"Briefing",
	"Scaffold",
	"Metal Flowers",
	"Exposure",
};

static char* p90SkinNames[]{
	"Default",
	"Leather",
	"Virus",
	"Storm",
	"Cold Blooded",
	"Glacier Mesh",
	"Sand Spray",
	"Death by Kitty",
	"Ash Wood",
	"Fallout Warning",
	"Scorched",
	"Emerald Dragon",
	"Teardown",
	"Blind Spot",
	"Trigon",
	"Desert Warfare",
	"Module",
	"Asiimov",
	"Shapewood",
	"Elite Build",
	"Chopper",
	"Grim",
	"Shallow Grave",
};

static char* bizonSkinNames[]{
	"Default",
	"Photic Zone",
	"Blue Streak",
	"Modern Hunter",
	"Forest Leaves",
	"Carbon Fiber",
	"Sand Dashed",
	"Urban Dashed",
	"Brass",
	"Irradiated Alert",
	"Rust Coat",
	"Water Sigil",
	"Night Ops",
	"Cobalt Halftone",
	"Harvester",
	"Antique",
	"Osiris",
	"Chemical Green",
	"Fuel Rod",
	"Bamboo Print",
	"Judgement of Anubis",
	"Jungle Slipstream",
	"High Roller",
};

static char* knife_skins[]{
	"Default",
	"Safari Mesh",
	"Boreal Forest",
	"Doppler Phase 1",
	"Doppler Phase 2",
	"Doppler Phase 3",
	"Doppler Phase 4",
	"Ruby",
	"Sapphire",
	"Black Pearl",
	"Slaughter",
	"Fade",
	"Crimson Web",
	"Night",
	"Blue Steel",
	"Stained",
	"Case Hardened",
	"Ultraviolet",
	"Urban Masked",
	"Damascus Steel",
	"Scorched",
	"Bright Water",
	"Emerald",
	"Gamma Doppler Phase 1",
	"Gamma Doppler Phase 2",
	"Gamma Doppler Phase 3",
	"Gamma Doppler Phase 4",
	"Freehand",
	"Tiger Tooth",
	"Rust Coat",
	"Marble Fade",
	"Autotronic",
	"Black Laminate",
	"Lore",
	"Forest DDPAT",
};

class Skin
{
public:
	int m_nFallbackPaintKit;
	int m_nFallbackSeed;
	int m_nFallbackStatTrak;
	float m_flFallbackWear;
};

static char* keyNames[] =
{
	"",
	"Mouse 1",
	"Mouse 2",
	"Middle Mouse",
	"Mouse 4",
	"Mouse 5",
	"Shift",
	"Space",
	"Insert",
};

class Config
{
public:

    // 
    // VISUALS
    // 
	OPTION(bool, visuals_fov_circle, false);
    OPTION(bool, esp_enabled,          false);
    OPTION(bool, esp_enemies_only,     false);
	OPTION(bool, esp_flash_check, false);
    OPTION(bool, esp_player_boxes,     false);
    OPTION(bool, esp_player_names,     false);
    OPTION(bool, esp_player_health,    false);
	OPTION(bool, esp_player_armour,    false);
    OPTION(bool, esp_player_weapons,   false);
	OPTION(int, esp_player_weapons_type, ICONS);
    OPTION(bool, esp_player_snaplines, false);
    OPTION(bool, esp_crosshair,        false);
    OPTION(bool, esp_dropped_weapons,  false);
    OPTION(bool, esp_defuse_kit,       false);
    OPTION(bool, esp_planted_c4,       false);
	OPTION(bool, skeleton_shit, false);

	OPTION(bool, esp_planted_c4_boxes, false);
	OPTION(bool, esp_planted_c4_text, false);
	OPTION(bool, esp_defuse_kit_boxes, false);
	OPTION(bool, esp_defuse_kit_text, false);
	OPTION(bool, esp_dropped_weapons_boxes, false);
	OPTION(bool, esp_dropped_weapons_text, false);
	OPTION(bool, visuals_no_aimpunch, false);
	OPTION(bool, watermarks, true);
	OPTION(bool, noflash, false);

    OPTION(bool, glow_enabled,      false);
    OPTION(bool, glow_enemies_only, false);
	OPTION(int, glow_style, GLOWTYPE_OUTLINE);
    OPTION(bool, glow_players,      false);
    OPTION(bool, glow_chickens,     false);
    OPTION(bool, glow_c4_carrier,   false);
    OPTION(bool, glow_planted_c4,   false);
    OPTION(bool, glow_defuse_kits,  false);
    OPTION(bool, glow_weapons,      false);

    OPTION(bool, chams_player_enabled,      false);
	OPTION(bool, chams_player_enemies_only, false);
	OPTION(bool, chams_localplayer, false);
    OPTION(bool, chams_player_wireframe,    false);
	OPTION(bool, chams_player_xqz, false);
	OPTION(int, chams_player_mode, CHAM_TYPE_MATERIAL);
    OPTION(bool, chams_arms_enabled,        false);
    OPTION(bool, chams_arms_wireframe,      false);
	OPTION(bool, chams_arms_xqz, false);
	OPTION(bool, chams_translucent, false);
	OPTION(int, chams_arms_mode, CHAM_TYPE_MATERIAL);

	OPTION(bool, nightmode, false);
	OPTION(bool, disable_post_processing, false);
	OPTION(bool, grenade_path_esp, false);
	OPTION(bool, hitmarkers, false);
	OPTION(int, hitmarkers_sound, false);
	OPTION(bool, misc_no_hands, false);
	OPTION(int, viewmodel_fov, 0);
	OPTION(int, fov, 0);
	OPTION(float, mat_ambient_light_r, 0.0f);
	OPTION(float, mat_ambient_light_g, 0.0f);
	OPTION(float, mat_ambient_light_b, 0.0f);
	OPTION(bool, drugs, false);
	OPTION(bool, paper, false);
	OPTION(bool, lowrestextures, false);
	OPTION(int, sky, 0);
	OPTION(bool, spec_list, false);
	OPTION(bool, spec_list_player_only, false);
	OPTION(int, pistol_backtracking_ticks, 12);
	OPTION(int, rifle_backtracking_ticks, 12);
	OPTION(int, sniper_backtracking_ticks, 12);
	OPTION(int, shotgun_backtracking_ticks, 12);
	OPTION(int, smg_backtracking_ticks, 12);

    //
    // MISC
    //
	OPTION(bool, autoaccept, false);
	OPTION(bool, thirdperson, false);
	OPTION(int, thirdpersonrange, 120);
	OPTION(bool, rank_reveal, false);
	OPTION(bool, misc_bhop_key, 0);
	OPTION(bool, memewalk, false);
	OPTION(int, name, KAWAII_EXPOSED);
	OPTION(int, clantag, DISABLED_CLANTAG);
	OPTION(int, airstuck_key, 0);
	OPTION(int, autostrafer, AUTOSTRAFER_NONE);
	OPTION(int, chatspam, chatspam_none);

	OPTION(bool, antiuntrusted, true);
	OPTION(bool, legit_antiaim, false);

	//
	// SKINS
	//
	// KNIVES
	OPTION(int, knife, KNIFE_DEFAULT);
	OPTION(int, knife_skin, 0);
	OPTION(int, player_model, 0);
	OPTION(int, knife_model, 0);;
	// GLOVES
	OPTION(int, glove, 0);
	OPTION(int, glove_skin, 0);
	OPTION(bool, test, false);
	// SKINS
	/* RIFLE */
	OPTION(int, ak47, DEFAULT_AK47SKIN);
	OPTION(int, galil, 0);
	OPTION(int, ssg08, 0);
	OPTION(int, sg553, 0);
	OPTION(int, awp, 0);
	OPTION(int, g3sg1, 0);
	OPTION(int, famas, 0);
	OPTION(int, m4a4, 0);
	OPTION(int, m4a1s, 0);
	OPTION(int, aug, 0);
	OPTION(int, scar20, 0);
	/* Heavy */
	OPTION(int, nova, 0);
	OPTION(int, xm1014, 0);
	OPTION(int, mag7, 0);
	OPTION(int, sawedoff, 0);
	OPTION(int, m249, 0);
	OPTION(int, negev, 0);
	/* Pistols */
	OPTION(int, p2000, 0);
	OPTION(int, usps, 0);
	OPTION(int, glock, 0);
	OPTION(int, elites, 0);
	OPTION(int, p250, 0);
	OPTION(int, fiveseven, 0);
	OPTION(int, deagle, 0);
	OPTION(int, cz75a, 0);
	OPTION(int, tec9, 0);
	OPTION(int, revolver, 0);
	/* SMGs */
	OPTION(int, mp9, 0);
	OPTION(int, mac10, 0);
	OPTION(int, mp7, 0);
	OPTION(int, ump45, 0);
	OPTION(int, p90, 0);
	OPTION(int, bizon, 0);

	// AIMBOT / HVH
	OPTION(float, antiaim_edge_dist, 0.0f);
	OPTION(bool, antiaim_antiresolver, false);
	OPTION(int, antiaim_pitch, ANTIAIM_PITCH_NONE);
	OPTION(int, antiaim_yaw, ANTIAIM_YAW_NONE);
	OPTION(int, antiaim_yaw_fake, ANTIAIM_YAW_NONE);
	OPTION(int, antiaim_thirdperson_angle, ANTIAIM_THIRDPERSON_REAL);
	OPTION(float, antiaim_spin_speed, 0.0f);
	OPTION(bool, antiaim_knife, false);
	OPTION(int, tankAntiaimKey, 0);
	//OPTION(bool, aimbot_rage, false);
	OPTION(bool, vischeck, false);
	OPTION(bool, no_recoil, false);
	OPTION(float, rcs_amount, 0.0f);
	OPTION(int, fakelag_amount, 0);
	OPTION(bool, fakelag_adaptive, false);
	OPTION(int, fakewalk_key, 0);
	OPTION(bool, resolver, false);
	OPTION(bool, backtracking, false);
	OPTION(bool, backtracking_tracer, false);
	OPTION(float, hitchance, 0.0f);
	OPTION(int, hitscan_amount, HITSCAN_NONE);

	// AIMBOT WEAPON CONFIGS 
	OPTION(int, pistol_aimbot_key, 0);
	OPTION(bool, pistol_vischeck, false);
	OPTION(bool, pistol_aimbot_silent, false);
	OPTION(bool, pistol_backtracking, false);
	OPTION(float, pistol_aimbot_AimbotFOV, 0.0f);
	OPTION(float, pistol_aimbot_smoothness, 0.0f);
	OPTION(float, pistol_rcs_amount, 0.0f);
	OPTION(int, pistol_legit_aimspot, 0);
	OPTION(bool, pistol_standalone_rcs, false);

	OPTION(int, rifle_aimbot_key, 0);
	OPTION(bool, rifle_vischeck, false);
	OPTION(bool, rifle_aimbot_silent, false);
	OPTION(bool, rifle_backtracking, false);
	OPTION(float, rifle_aimbot_AimbotFOV, 0.0f);
	OPTION(float, rifle_aimbot_smoothness, 0.0f);
	OPTION(float, rifle_rcs_amount, 0.0f);
	OPTION(int, rifle_legit_aimspot, 0);
	OPTION(bool, rifle_standalone_rcs, false);

	OPTION(int, sniper_aimbot_key, 0);
	OPTION(bool, sniper_vischeck, false);
	OPTION(bool, sniper_aimbot_silent, false);
	OPTION(bool, sniper_backtracking, false);
	OPTION(float, sniper_aimbot_AimbotFOV, 0.0f);
	OPTION(float, sniper_aimbot_smoothness, 0.0f);
	OPTION(float, sniper_rcs_amount, 0.0f);
	OPTION(int, sniper_legit_aimspot, 0);
	OPTION(bool, sniper_standalone_rcs, false);

	OPTION(int, shotgun_aimbot_key, 0);
	OPTION(bool, shotgun_vischeck, false);
	OPTION(bool, shotgun_aimbot_silent, false);
	OPTION(bool, shotgun_backtracking, false);
	OPTION(float, shotgun_aimbot_AimbotFOV, 0.0f);
	OPTION(float, shotgun_aimbot_smoothness, 0.0f);
	OPTION(float, shotgun_rcs_amount, 0.0f);
	OPTION(int, shotgun_legit_aimspot, 0);
	OPTION(bool, shotgun_standalone_rcs, false);

	OPTION(int, smg_aimbot_key, 0);
	OPTION(bool, smg_vischeck, false);
	OPTION(bool, smg_aimbot_silent, false);
	OPTION(bool, smg_backtracking, false);
	OPTION(float, smg_aimbot_AimbotFOV, 0.0f);
	OPTION(float, smg_aimbot_smoothness, 0.0f);
	OPTION(float, smg_rcs_amount, 0.0f);
	OPTION(int, smg_legit_aimspot, 0);
	OPTION(bool, smg_standalone_rcs, false);

	OPTION(float, aimbot_AimbotFOV, 0.0f);
	OPTION(float, aimbot_smoothness, 0.0f);
	OPTION(bool, aimbot_silent, false);
	OPTION(bool, autoshoot, false);
	OPTION(bool, autoscope, false);
	OPTION(int, legit_aimspot, 0);
	OPTION(int, aimbot_key, 0);
	OPTION(bool, autowall, false);
	OPTION(float, autowall_min_damage, 1.0f);

	// TRIGGERBOT
	OPTION(bool, triggerbotactive, false);
	OPTION(int, triggerbotkey, 0);
	OPTION(int, triggerbot_spot, 0);
	

    // 
    // COLORS
    // 

    OPTION(Color, color_esp_ally_visible, Color(255, 0, 160, 255));
    OPTION(Color, color_esp_enemy_visible, Color(255, 0, 160, 255));
    OPTION(Color, color_esp_ally_occluded, Color(0, 255, 255, 255));
    OPTION(Color, color_esp_enemy_occluded, Color(0, 255, 255, 255));
    OPTION(Color, color_esp_weapons, Color(0, 255, 255, 255));
    OPTION(Color, color_esp_defuse, Color(0, 255, 255, 255));
    OPTION(Color, color_esp_c4, Color(0, 255, 255, 255));

    OPTION(Color, color_glow_ally,                   Color(0, 255, 255, 200));
    OPTION(Color, color_glow_enemy,                  Color(255, 0, 255, 200));
    OPTION(Color, color_glow_chickens,               Color(255, 255, 255, 200));
    OPTION(Color, color_glow_c4_carrier,             Color(255, 200, 0, 200));
    OPTION(Color, color_glow_planted_c4,             Color(255, 200, 0, 200));
    OPTION(Color, color_glow_defuse,                 Color(0, 0, 255, 200));
    OPTION(Color, color_glow_weapons,                Color(0, 255, 0, 255));

    OPTION(Color, color_chams_player_ally_visible,   Color(255, 0, 160, 255));
    OPTION(Color, color_chams_player_ally_occluded,  Color(0, 255, 255, 255));
    OPTION(Color, color_chams_player_enemy_visible,  Color(255, 0, 160, 255));
    OPTION(Color, color_chams_player_enemy_occluded, Color(0, 255, 255, 255));
    OPTION(Color, color_chams_arms_visible,			 Color(255, 0, 160, 255));
    OPTION(Color, color_chams_arms_occluded,		 Color(255, 0, 160, 255));

	//
	// MENU
	//
	OPTION(int, curTab, 0);
	OPTION(int, curTab_aimbot, 0);
	//OPTION(Color, menu_color, Color(43, 40, 43, 240));
	//OPTION(Color, menu_accents_color, Color(244, 66, 241, 220));
	//OPTION(Color, menu_text_color, Color(244, 66, 241, 220));
	//OPTION(Color, menu_outline_color, Color(255, 255, 255, 100));
	//OPTION(Color, menu_hover, Color(27, 26, 27, 220));
	//OPTION(float, menu_roundness, 3.00f);
	OPTION(Color, color_menu_main, Color(98, 191, 87, 255));
	OPTION(Color, color_menu_body, Color(12, 12, 12, 255));
	OPTION(Color, color_menu_font, Color(240, 240, 240, 240));

	OPTION(float, menu_fade_speed, 1.0f);
};

extern Config g_Options;
extern bool   g_Unload;