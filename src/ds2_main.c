//ds2_main.c
#include <libBAG.h>
#include "animations.h"
#include "quick2dEngine.h"

//important file paths
const char RootDir[] = "/space invaders/";
const char SkinDir[] = "skins/";
const char InvadersLayout[] = "invaders.tbag";

/*//===============================================
Template object
//==============================================
typedef struct Object_t{
    GFXObj_t *gfx;
    Point_t Pos;

    void (*reset)(struct Bullet_t *);
    void (*update)(struct Bullet_t *, int);
    void (*setXY)(struct Bullet_t *, int, int);
    void (*draw)(unsigned short *, struct Bullet_t *);
}Object_t;


static void objectSetXY(Bullet_t *obj, int x, int y){
    Point_t *pos = &obj->Pos;
    (*pos->getX(pos)) = norm_fix(x);
    (*pos->getY(pos)) = norm_fix(y);
}

static void objectReset(Object_t *obj){
    //set objects initial valus
    Point_t *pos = &obj->Pos;
    pos->speed = 0;//no moving
    pos->x = -256;//move offscreen
    pos->y = -256;
}

static void objectUpdate(Object_t *obj, int update){
    //update object here (controls or collisions)
    obj->Pos.update(&obj->Pos);
}

static void objectDraw(unsigned short *dest, Object_t *obj){
    GFXObj_t *gfx = bullet->gfx;
    Point_t *pos = &bullet->Pos;
    BAG_Display_SetGfxBlitXY(gfx, fix_norm(*pos->getX(pos)), fix_norm(*pos->getY(pos)));
    BAG_Display_DrawObjSlowEx(gfx, dest, GAME_WIDTH, GAME_HEIGHT);
}

static void objectInit(Object_t *obj, GFXObj_t *gfx){
    //initialize object
    memset(obj, 0, sizeof(Object_t));
    obj->gfx = gfx;
    initPoint(&obj->Pos);
    //assign functions
    obj->setXY = (void*)&objectSetXY;
    obj->reset = (void*)&objectReset;
    obj->update = (void*)&objectUpdate;
    obj->draw = (void*)&objectDraw;
    objectReset(obj);
}
*/



/*==========================================================================
Game stuff
==========================================================================*/

#define BULLET_SPEED 1024
#define PLAYER_SPEED 512
#define ALIEN_SPEED 128


#define ALIEN_GRID_WD 11
#define DEAD_FRAME 7
#define NUM_BUNKERS 4
#define DIST_BETWEEN_BUNKERS 42


typedef struct Bullet_t{
    GFXObj_t *gfx;
    Point_t Pos;

    char (*collisionObj)(struct Bullet_t *, GFXObj_t *);
    char (*collisionArea)(struct Bullet_t *, int, int, int, int);
    void (*shoot)(struct Bullet_t *, int, int);
    void (*reset)(struct Bullet_t *);
    void (*update)(struct Bullet_t *, int);
    void (*setXY)(struct Bullet_t *, int, int);
    void (*draw)(unsigned short *, struct Bullet_t *);
}Bullet_t;

static char bulletCollisionObj(Bullet_t * bullet, GFXObj_t *target){
    return obj_collision_PtObj(&bullet->Pos, bullet->gfx, target);
}

static char bulletCollisionArea(Bullet_t *bullet, int x1, int y1, int x2, int y2){
    GFXObj_t *gfx = bullet->gfx;
    return (*BAG_Display_GetGfxBlitX(gfx) >= x1 && *BAG_Display_GetGfxBlitX(gfx) <= x2 &&
            *BAG_Display_GetGfxBlitY(gfx) >= y1 && *BAG_Display_GetGfxBlitY(gfx) <= y2);
}

static void bulletSetXY(Bullet_t *bullet, int x, int y){
    Point_t *pos = &bullet->Pos;
    (*pos->getX(pos)) = norm_fix(x);
    (*pos->getY(pos)) = norm_fix(y);
}

static void bulletShoot(Bullet_t *bullet, int speed, int angle){
    Point_t *pos = &bullet->Pos;
    pos->speed = speed;
    pos->angle = angle;
}

static void bulletReset(Bullet_t *bullet){
    Point_t *pos = &bullet->Pos;
    pos->speed = 0;//no moving
    pos->x = -256;//move offscreen
    pos->y = -256;
}

static void bulletUpdate(Bullet_t *bullet, int update){
    //delete bullet if offscreen or hits
    if(!update || !bullet->Pos.onScreen(&bullet->Pos)){
        bulletReset(bullet);
        return;
    }
    bullet->Pos.update(&bullet->Pos);
}

static void bulletDraw(unsigned short *dest, Bullet_t *bullet){
    GFXObj_t *gfx = bullet->gfx;
    Point_t *pos = &bullet->Pos;
    BAG_Display_SetGfxBlitXY(gfx, fix_norm(*pos->getX(pos)), fix_norm(*pos->getY(pos)));
    BAG_Display_DrawObjSlowEx(gfx, dest, GAME_WIDTH, GAME_HEIGHT);
}

static void bulletInit(Bullet_t *bullet, GFXObj_t *gfx){
    memset(bullet, 0, sizeof(Bullet_t));

    bullet->gfx = gfx;
    initPoint(&bullet->Pos);

    bullet->collisionObj = (void*)&bulletCollisionObj;
    bullet->collisionArea = (void*)&bulletCollisionArea;
    bullet->shoot = (void*)&bulletShoot;
    bullet->reset = (void*)&bulletReset;
    bullet->update = (void*)&bulletUpdate;
    bullet->draw = (void*)&bulletDraw;
    bullet->setXY = (void*)&bulletSetXY;
    bulletReset(bullet);
}


/*==========================================================================
Ship Info
==========================================================================*/
typedef struct Player_t{
    GFXObj_t *gfx, *laser_gfx;
    AnimData DeathAnim;

    Point_t Pos;
    Bullet_t Laser;

    unsigned int score;
    char shot, lives, hit;

    void (*update)(struct Player_t *, void (*extra)(void));
    void (*draw) (unsigned short *, struct Player_t *);
    void (*reset)(struct Player_t *);
    void (*resetPos)(struct Player_t *);
    void (*resetShot)(struct Player_t *);
}Player_t;

static void playerResetPos(Player_t *p){
    Point_t *pos = &p->Pos;
    (*pos->getX(pos)) = norm_fix( (SCREEN_WIDTH - (*BAG_Display_GetGfxScaleWd(p->gfx)))>>1 );
    (*pos->getY(pos)) = norm_fix(GAME_HEIGHT - 12);
    (*pos->getSpeed(pos)) = 0;

    BAG_Display_SetObjFrame(p->gfx, FRAME_HOR, 0);
    p->Laser.reset(&p->Laser);
}

static void playerResetShot(Player_t *p){
    p->shot = 0;
}

static void playerReset(Player_t *p){
    p->lives = 3;
    p->score = 0;
    playerResetPos(p);
    playerResetShot(p);
    p->Laser.reset(&p->Laser);
    Animation_ResetProfile(&p->DeathAnim);
}

static void playerUpdate(Player_t *p, void (*extra)(void)){
    //left and right movement
    Point_t *pos = &p->Pos;
    if(p->hit){
        Animation_RunProfile(&p->DeathAnim);
        if(!p->DeathAnim.done)//wait till animation is finished
            return;//exit function when hit

        playerResetPos(p);
        playerResetShot(p);
        p->Laser.reset(&p->Laser);
        Animation_ResetProfile(&p->DeathAnim);
        p->lives--;
        p->hit = 0;
    }



    (*pos->getSpeed(pos)) = 0;//no movement when no pads are held
    if(Pad.Held.Left){
        (*pos->getSpeed(pos)) = PLAYER_SPEED;
        (*pos->getAngle(pos)) = ANGLE_LEFT;
    }
    else if(Pad.Held.Right){
        (*pos->getSpeed(pos)) = PLAYER_SPEED;
        (*pos->getAngle(pos)) = ANGLE_RIGHT;       
    }

    //laser
    if(Pad.Newpress.A && !p->shot){
        //center bullet over player ship
        int pX = fix_norm(*pos->getX(pos)) + ((*BAG_Display_GetGfxFrameWd(p->gfx))>>1);
        int pY = fix_norm(*pos->getY(pos));

        p->Laser.setXY(&p->Laser, pX, pY);
        p->Laser.shoot(&p->Laser, BULLET_SPEED, ANGLE_UP);
        p->shot++;
    }
    //update bullet if it is moving
    p->Laser.update(&p->Laser, p->shot);
    p->shot = ((*p->Laser.Pos.getSpeed(&p->Laser.Pos)) > 0);

    if(extra != NULL)
        extra();
    //update player movement
    pos->update(pos);
}

static void playerDraw(unsigned short *dest, Player_t *p){
    //draw player sprite
    GFXObj_t *gfx = p->gfx;
    Point_t *pos = &p->Pos;
    BAG_Display_SetGfxBlitXY(gfx, fix_norm(*pos->getX(pos)), fix_norm(*pos->getY(pos)));
    BAG_Display_DrawObjSlowEx(gfx, dest, GAME_WIDTH, GAME_HEIGHT);  

    //draw player bullet
    if(p->shot)
        p->Laser.draw(dest, &p->Laser);
}


void Player_Init(Player_t *p, GFXObj_t *sprite_gfx, GFXObj_t *laser_gfx){
    memset(p, 0, sizeof(Player_t));

    p->gfx = sprite_gfx;
    p->laser_gfx = laser_gfx;

    initPoint(&p->Pos);
    bulletInit(&p->Laser, laser_gfx);

    p->update = (void*)&playerUpdate;
    p->draw = (void*)&playerDraw;
    p->reset = (void*)playerReset;
    p->resetPos = (void*)playerResetPos;
    p->resetShot = (void*)playerResetShot;
    

    //death animation
    AnimData tempAnim = {
        sprite_gfx,/*gfx*/ 1,/*first frame*/ 2,/*last frame*/ 1,/*idle frame*/ 128,/*speed*/
        256,/*frames*/ 6,/*loop*/ 0,/*done*/ 0,/*vertical offset*/ 0,/*timer*/ 0,/*loopTimes*/
    };
    memcpy(&p->DeathAnim, &tempAnim, sizeof(AnimData));

    playerReset(p);
}


/*==========================================================================
Invaders
==========================================================================*/
typedef struct Aliens_t{
    TiledBG_t *gfx;
    GFXObj_t *laser_gfx;

    Point_t Pos;
    Bullet_t Laser[ALIEN_GRID_WD];
    char shot[ALIEN_GRID_WD];
    char hitPlayer;

    void (*update)(struct Aliens_t *, void (*extra)(void));
    void (*draw) (unsigned short *, struct Aliens_t *);
    void (*reset)(struct Aliens_t *);
    void (*resetPos)(struct Aliens_t *);
    void (*resetShot)(struct Aliens_t *);
}Aliens_t;


static void alienDraw(unsigned short *dest, Aliens_t *a){
    Point_t *pos = &a->Pos;
    BAG_TileBG_DrawBGEx(dest, a->gfx, fix_norm(*pos->getX(pos)), fix_norm(*pos->getY(pos)), GAME_WIDTH, GAME_HEIGHT);

    for(int i = 0; i < ALIEN_GRID_WD; i++){
        if(a->shot[i])
            a->Laser[i].draw(dest, &a->Laser[i]);
    }
}


static void alienResetPos(Aliens_t *a){
    Point_t *pos = &a->Pos;
    TiledBG_t *gfx = a->gfx;
    (*pos->getX(pos)) = norm_fix((GAME_WIDTH - (gfx->width * gfx->tileWd))>>1);
    (*pos->getY(pos)) = 0;
    (*pos->getSpeed(pos)) = ALIEN_SPEED;
    (*pos->getAngle(pos)) = ANGLE_LEFT;
}

static void alienResetShots(Aliens_t *a){
    for(int i = 0; i < ALIEN_GRID_WD; i++){
        a->shot[i] = 0;
        a->Laser[i].reset(&a->Laser[i]);
    }
}

static void alienReset(Aliens_t *a){
    //set animations
    BAG_TileBG_SetTileAnimation(a->gfx, 1, 1, 64, 512, 0);
    BAG_TileBG_SetTileAnimation(a->gfx, 3, 1, 64, 512, 0);
    BAG_TileBG_SetTileAnimation(a->gfx, 5, 1, 64, 512, 0);
    //reset alien positions
    alienResetPos(a);
    //reset alien bullets
    alienResetShots(a);
}

static void alienUpdate(Aliens_t *a, void (*extra)(void)){
    if(a->hitPlayer){
        alienResetShots(a);
        return;
    }

    Point_t *pos = &a->Pos;

    if(fix_norm(*pos->getX(pos)) <= 0 || (fix_norm(*pos->getX(pos)) + (a->gfx->width * a->gfx->tileWd)) >= GAME_WIDTH){
        (*pos->getAngle(pos)) = FLIP_ANGLE(*pos->getAngle(pos));
        (*pos->getY(pos)) += norm_fix(a->gfx->tileHt);
    }
    pos->update(pos);

    //update lasers
    for(int i = 0; i < ALIEN_GRID_WD; i++){

        //update laser shot on this column
        if(a->shot[i]){
            a->Laser[i].update(&a->Laser[i], a->shot[i]);
            a->shot[i] = ((*a->Laser[i].Pos.getSpeed(&a->Laser[i].Pos)) > 0);
        }
        else{//check if an alien randomly shoots
            for(int y = 5; y >= 0; y--){
                //check if alien is alive in slot
                if(BAG_TileBG_GetTile(a->gfx, i, y) <= 0 || BAG_TileBG_GetTile(a->gfx, i, y) >= DEAD_FRAME)
                    continue;

                //chance for shot
                if((BAG_Math_rand() % 257) != 1)
                    break;//no shot on this column

                //center bullet over alien
                int aX = fix_norm(*pos->getX(pos)) + ((i + 1) * a->gfx->tileWd) - (a->gfx->tileWd >> 1);
                int aY = fix_norm(*pos->getY(pos)) + ((y + 1) * a->gfx->tileHt);

                a->Laser[i].setXY(&a->Laser[i], aX, aY);
                a->Laser[i].shoot(&a->Laser[i], BULLET_SPEED, ANGLE_DOWN);
                a->shot[i]++;
                break;
            }
        }
    }
    //update laser animation
    BAG_Display_UpdateAnim(a->laser_gfx, 0, 1, 128, 512, FRAME_HOR);


    if(extra != NULL)
        extra();
}

void Alien_Init(Aliens_t *a, TiledBG_t *bg, GFXObj_t *laser_gfx){
    memset(a, 0, sizeof(Aliens_t));

    a->gfx = bg;
    a->laser_gfx = laser_gfx;
    initPoint(&a->Pos);
    for(int i = 0; i < ALIEN_GRID_WD; i++)
        bulletInit(&a->Laser[i], laser_gfx);

    a->update = (void*)&alienUpdate;
    a->draw = (void*)&alienDraw;
    a->reset = (void*)alienReset;
    a->resetPos = (void*)alienResetPos;
    a->resetShot = (void*)alienResetShots;

    alienReset(a);
}

/*==========================================================================
Logic
==========================================================================*/
TiledBG_t Invaders;//aliens tiled background

static GFXObj_t Canvas,//main buffer to blit to
         Ship,
         Bunker[NUM_BUNKERS],
         Bullets,
         EnemyBullets,
         BonusAlien,
         Background;


static Player_t Player = {0};
static Aliens_t Aliens = {0};


void loadInvaders(const char *curSkin){
    char path[MAX_PATH<<1];
    memset(&path, 0, sizeof(path));
    
    sprintf(path, "%s%s", RootDir, InvadersLayout);
    path[MAX_PATH] = '\0';

    sprintf(&path[MAX_PATH+1], "%s%s%s/invaders", RootDir, SkinDir, curSkin);
    BAG_TileBG_LoadBG(&path[MAX_PATH+1], path, &Invaders);
    BAG_TileBG_SetProperties(&Invaders, GAME_HEIGHT >> Invaders.divY, GAME_WIDTH >> Invaders.divX, 0, 0);
    Invaders.forceMode = 1;
}

void LoadGraphics(const char *curSkin){
    loadInvaders(curSkin);
    char path[MAX_PATH];
    memset(path, 0, sizeof(path));

    sprintf(path, "%s%s%s/ship", RootDir, SkinDir, curSkin);
    BAG_Display_LoadObjExt(path, &Ship);
    //BAG_Display_SetGfxWidthHeight(&Ship, 16, 8);
    BAG_Display_SetGfxFrameDim(&Ship, 16, 8);

    sprintf(path, "%s%s%s/bunker", RootDir, SkinDir, curSkin);
    for(int i = 0; i < NUM_BUNKERS; i++)
        BAG_Display_LoadObjExt(path, &Bunker[i]);

    sprintf(path, "%s%s%s/bullets", RootDir, SkinDir, curSkin);
    BAG_Display_LoadObjExt(path, &Bullets);
    BAG_Display_SetGfxFrameDim(&Bullets, 4, 7);

    sprintf(path, "%s%s%s/bullets", RootDir, SkinDir, curSkin);
    BAG_Display_LoadObjExt(path, &EnemyBullets);
    BAG_Display_SetGfxFrameDim(&EnemyBullets, 4, 7);
    BAG_Display_SetObjFrame(&EnemyBullets, FRAME_VERT, 1);
    BAG_Display_StartAnim(&EnemyBullets);

    sprintf(path, "%s%s%s/bonus", RootDir, SkinDir, curSkin);
    BAG_Display_LoadObjExt(path, &BonusAlien);

    sprintf(path, "%s%s%s/background", RootDir, SkinDir, curSkin);
    BAG_Display_LoadObjExt(path, &Background);
}




void Flip_Screen(GFXObj_t *screen){
    BAG_Display_SetGfxFrameDim(screen, SCREEN_WIDTH, SCREEN_HEIGHT);

    //set top screen
    BAG_Display_SetObjFrame(screen, FRAME_VERT, 0);
    BAG_Display_DrawObjFast(screen, up_screen_addr, 0, 0);

    //set top screen
    BAG_Display_SetObjFrame(screen, FRAME_VERT, 1);
    BAG_Display_DrawObjFast(screen, down_screen_addr, 0, 0);

    BAG_Display_SetGfxFrameDim(screen, GAME_WIDTH, GAME_HEIGHT);
    //flip screens
    ds2_flipScreen(DUAL_SCREEN, 1);
}


void DrawScreen(GFXObj_t *screen){
    unsigned short *Screen_Buffer = BAG_Display_GetGfxBuf(screen);
    if(Screen_Buffer == NULL){
        printf("screen buffer error!\n");
        while(1);
    }  
    BAG_Display_DrawObjFastEx(&Background, Screen_Buffer, GAME_WIDTH, GAME_HEIGHT);

    //draw bunkers
    int bunkerStartX = 21,
        bunkerStartY = 330;
    for(int i = 0; i < NUM_BUNKERS; i++){
        BAG_Display_SetGfxBlitXY(&Bunker[i], 
                                bunkerStartX + (i*((*BAG_Display_GetGfxFrameWd(&Bunker[i])) + DIST_BETWEEN_BUNKERS)),
                                bunkerStartY);
        BAG_Display_DrawObjSlowEx(&Bunker[i], Screen_Buffer, GAME_WIDTH, GAME_HEIGHT);          
    }

    Player.draw(Screen_Buffer, &Player);
    Aliens.draw(Screen_Buffer, &Aliens);
    Flip_Screen(&Canvas);
}

static void DrawBunkerHit(GFXObj_t *bunker, int x, int y){
    unsigned short *buf = BAG_Display_GetGfxBuf(bunker);
    char hole[5][5] = {
        {1,1,1,1,1},
        {0,1,1,1,0},
        {1,1,1,1,1},
        {0,1,1,1,0},
        {0,0,1,0,0},
    };
    //draw hole
    for(int x1= 0; x1 < 5; x1++){
        for(int y1 = 0; y1 < 5; y1++){
            if(!hole[x1][y1])
                continue;
            BAG_Draw_BlitPixel(buf, 
                                *BAG_Display_GetGfxFrameWd(bunker), 
                                *BAG_Display_GetGfxFrameHt(bunker), 
                                x + x1, y + y1, MAGENTA);
        }
    }

}



/*
Bullet collision for the players shot
-need to add bunker collisions
*/
static void PlayerBulletCollision(void){

    if(!Player.shot)
        return;

    //get alien information
    Point_t *aPos = &Aliens.Pos;
    TiledBG_t *aGfx = Aliens.gfx;

    unsigned int *tiles[4][3];
    if(GET_FLAG(obj_collisionTile_Pt(&Player.Laser.Pos, Player.laser_gfx, aPos, aGfx, tiles), COLLISION_UP)){
        if(tiles[0][1]){
            if(*tiles[0][1] > 0 && *tiles[0][1] < DEAD_FRAME){//top center tile above bullet
                *tiles[0][1] = DEAD_FRAME;
                BAG_TileBG_SetTileAnimation(aGfx, DEAD_FRAME, 1, 32, 512, 1);
                Player.resetShot(&Player);
                tiles[0][1] = NULL;
            }
        }
    }
}



/*
Misc things for aliens such as:
-removal of dead aliens
-their bullet collisions with player and bunkers
*/
//sets all the dead aliens to tile 0, to prevent multiple death animations
static void clearDead(TiledBG_t *bg){
    TileAnimData *data = &bg->AnimData[DEAD_FRAME - 1];
    for(int x = 0; x < bg->width; x++){
        for(int y = 0; y < bg->height; y++){
            int tile = BAG_TileBG_GetTile(bg, x, y) + data->frameDiff;
            if(tile > DEAD_FRAME)
                BAG_TileBG_SetTile(bg, x, y, 0);
        }
    }
}

static void AlienMisc(void){
    clearDead(Aliens.gfx);

    if(Aliens.hitPlayer)//pause gameplay till player respawns
        return;

    for(int pew = 0; pew < ALIEN_GRID_WD; pew++){
        //check laser bunker collision
        for(int i = 0; i < NUM_BUNKERS; i++){
            //collision with the graphics
            if(Aliens.Laser[pew].collisionObj(&Aliens.Laser[pew], &Bunker[i])){

                //check for pixel collision at center bottom of bullet
                s16 bX = fix_norm(*Aliens.Laser[pew].Pos.getX(&Aliens.Laser[pew].Pos)) - (*BAG_Display_GetGfxBlitX(&Bunker[i]));
                s16 bY = fix_norm(*Aliens.Laser[pew].Pos.getY(&Aliens.Laser[pew].Pos)) - (*BAG_Display_GetGfxBlitY(&Bunker[i]));
                bX += ((*BAG_Display_GetGfxFrameWd(Aliens.laser_gfx))>>1);
                bY += *BAG_Display_GetGfxFrameHt(Aliens.laser_gfx);

                //check if pixel isn't transparent
                unsigned short col = BAG_Draw_GetPixel(BAG_Display_GetGfxBuf(&Bunker[i]), 
                                    *BAG_Display_GetGfxFrameWd(&Bunker[i]), 
                                    *BAG_Display_GetGfxFrameHt(&Bunker[i]), bX, bY);

                if( col != MAGENTA && col > 0){
                    DrawBunkerHit(&Bunker[i], bX, bY - 2);
                    Aliens.Laser[pew].reset(&Aliens.Laser[pew]);
                    Aliens.shot[pew] = 0;
                }
            }
        }

        //check player collision
        if(Aliens.Laser[pew].collisionObj(&Aliens.Laser[pew], &Ship)){
            Player.hit = 1;
            return;
        }
    }

}


/*
Back to normal programming stuff
*/
void update(void){
    Player.update(&Player, &PlayerBulletCollision);
    Aliens.hitPlayer = Player.hit;
    Aliens.update(&Aliens, &AlienMisc);
}



void ds2_main(void){
    if(!BAG_Init(1))
        ds2_plug_exit();

    BAG_Core_SetFPS(120);
    ds2_setCPUclocklevel(13);
    //play area
    BAG_Display_CreateObj(&Canvas, 16, GAME_WIDTH, GAME_HEIGHT, GAME_WIDTH, GAME_HEIGHT);
    printf("play screen created!\n");
    //load graphics
    LoadGraphics("default");
    printf("graphics loaded\n");

    //initiate player
    Player_Init(&Player, &Ship, &Bullets);
    printf("player initiated\n");

    //initiate aliens
    Alien_Init(&Aliens, &Invaders, &EnemyBullets);
    printf("enemies initiated\n");
    DrawScreen(&Canvas);

    while(1){
        update();
        DrawScreen(&Canvas);
        if(Pad.Newpress.L)
            BAG_Display_ScrnCap(DUAL_SCREEN, RootDir);

        BAG_Update();
    }
}


