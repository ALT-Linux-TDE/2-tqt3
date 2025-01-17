/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */

#ifndef __SPRITES_H__
#define __SPRITES_H__

#include <ntqcanvas.h>

#define ID_ROCK_LARGE           1024
#define ID_ROCK_MEDIUM          1025
#define ID_ROCK_SMALL           1026

#define ID_MISSILE              1030

#define ID_BIT                  1040
#define ID_EXHAUST              1041

#define ID_ENERGY_POWERUP       1310
#define ID_TELEPORT_POWERUP     1311
#define ID_BRAKE_POWERUP        1312
#define ID_SHIELD_POWERUP       1313
#define ID_SHOOT_POWERUP        1314

#define ID_SHIP                 1350
#define ID_SHIELD               1351

#define MAX_SHIELD_AGE          350
#define MAX_POWERUP_AGE         500
#define MAX_MISSILE_AGE         40

class KMissile : public TQCanvasSprite
{
public:
    KMissile( TQCanvasPixmapArray *s, TQCanvas *c ) : TQCanvasSprite( s, c )
        { myAge = 0; }

    virtual int rtti() const { return ID_MISSILE; }

    void growOlder() { myAge++; }
    bool expired() { return myAge > MAX_MISSILE_AGE; }

private:
    int myAge;
};

class KBit : public TQCanvasSprite
{
public:
    KBit( TQCanvasPixmapArray *s, TQCanvas *c ) : TQCanvasSprite( s, c )
	{  death = 7; }

    virtual int rtti() const {  return ID_BIT; }

    void setDeath( int d ) { death = d; }
    void growOlder() { death--; }
    bool expired() { return death <= 0; }

private:
    int death;
};

class KExhaust : public TQCanvasSprite
{
public:
    KExhaust( TQCanvasPixmapArray *s, TQCanvas *c ) : TQCanvasSprite( s, c )
	{  death = 1; }

    virtual int rtti() const {  return ID_EXHAUST; }

    void setDeath( int d ) { death = d; }
    void growOlder() { death--; }
    bool expired() { return death <= 0; }

private:
    int death;
};

class KPowerup : public TQCanvasSprite
{
public:
  KPowerup( TQCanvasPixmapArray *s, TQCanvas *c, int t ) : TQCanvasSprite( s, c ),
        myAge( 0 ), type(t) { }

  virtual int rtti() const { return type; }

  void growOlder() { myAge++; }
  bool expired() const { return myAge > MAX_POWERUP_AGE; }

protected:
  int myAge;
  int type;
};

class KRock : public TQCanvasSprite
{
public:
    KRock (TQCanvasPixmapArray *s, TQCanvas *c, int t, int sk, int st) : TQCanvasSprite( s, c )
        { type = t; skip = cskip = sk; step = st; }

    void nextFrame()
	{
	    if (cskip-- <= 0) {
		setFrame( (frame()+step+frameCount())%frameCount() );
		cskip = TQABS(skip);
	    }
	}

    virtual int rtti() const { return type; }

private:
    int type;
    int skip;
    int cskip;
    int step;
};

class KShield : public TQCanvasSprite
{
public:
  KShield( TQCanvasPixmapArray *s, TQCanvas *c )
      : TQCanvasSprite( s, c ) {}

  virtual int rtti() const { return ID_SHIELD; }
};

#endif
