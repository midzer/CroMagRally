//
// metaobjects.h
//

#ifndef __METAOBJECTS_H_
#define __METAOBJECTS_H_

#include "ogl_support.h"
#include <gl.h>

#define	MO_COOKIE				0xfeedface		// set at head of every object for validation
#define	MAX_MATERIAL_LAYERS		4				// max multitexture layers
#define	MO_MAX_MIPMAPS			6				// max mipmaps per material

#define	SPRITE_SCALE_BASIS_DENOMINATOR	640.0f

		/* OBJECT TYPES */
enum
{
	MO_TYPE_GROUP		= 	'grup',
	MO_TYPE_GEOMETRY	=	'geom',
	MO_TYPE_MATERIAL	=	'matl',
	MO_TYPE_MATRIX		=	'mtrx',
	MO_TYPE_PICTURE		=	'pict',
	MO_TYPE_SPRITE		=	'sprt'
};

	/* OBJECT SUBTYPES */

enum
{
	MO_GEOMETRY_SUBTYPE_VERTEXARRAY	=	'vary'
};


		/* META OBJECT HEADER */
		//
		// This structure is always at the head of any metaobject
		//

struct MetaObjectHeader
{
	u_long		cookie;						// this value should always == MO_COOKIE
	long		refCount;					// # times this is referenced
	u_long		type;						// object type
	u_long		subType;					// object sub-type
	void		*data;						// pointer to meta object's specific data

	struct MetaObjectHeader *parentGroup;			// illegal reference to parent group, or nil if no parent

	struct MetaObjectHeader *prevNode;			// ptr to previous node in linked list
	struct MetaObjectHeader *nextNode;			// ptr to next node in linked list
};
typedef struct MetaObjectHeader MetaObjectHeader;

typedef	void * MetaObjectPtr;


		/****************/
		/* GROUP OBJECT */
		/****************/

#define	MO_MAX_ITEMS_IN_GROUP	70

typedef struct
{
	int					numObjectsInGroup;
	MetaObjectHeader	*groupContents[MO_MAX_ITEMS_IN_GROUP];
}MOGroupData;

typedef struct
{
	MetaObjectHeader	objectHeader;
	MOGroupData			objectData;
}MOGroupObject;


		/*******************/
		/* MATERIAL OBJECT */
		/*******************/

typedef struct
{
	OGLSetupOutputType *setupInfo;					// materials are draw context relative, so remember which context we're using now

	u_long			flags;
	OGLColorRGBA	diffuseColor;					// rgba diffuse color

	u_long			numMipmaps;						// # texture mipmaps to use
	u_long			width,height;					// dimensions of texture
	GLint			pixelSrcFormat;					// OGL format (GL_RGBA, etc.) for src pixels
	GLint			pixelDstFormat;					// OGL format (GL_RGBA, etc.) for VRAM
	void			*texturePixels[MO_MAX_MIPMAPS]; // ptr to texture pixels for each mipmap
	GLuint			textureName[MO_MAX_MIPMAPS]; 	// texture name assigned by OpenGL
}MOMaterialData;

typedef struct
{
	MetaObjectHeader	objectHeader;
	MOMaterialData		objectData;
}MOMaterialObject;


		/********************************/
		/* VERTEX ARRAY GEOMETRY OBJECT */
		/********************************/

typedef struct
{
	GLuint vertexIndices[3];
}MOTriangleIndecies;

typedef struct
{
	int					numMaterials;						// # material layers used in geometry (if negative, then use current texture)
	MOMaterialObject 	*materials[MAX_MATERIAL_LAYERS];	// a reference to a material meta object

	int					numPoints;							// # vertices in the model
	int					numTriangles;						// # triangls in the model
	OGLPoint3D			*points;							// ptr to array of vertex x,y,z coords
	OGLVector3D			*normals;							// ptr to array of vertex normals
	OGLTextureCoord		*uvs;								// ptr to array of vertex uv's
	OGLColorRGBA_Byte	*colorsByte;						// ptr to array of vertex colors (byte & float versions)
	OGLColorRGBA		*colorsFloat;
	MOTriangleIndecies	*triangles;						// ptr to array of triangle triad indecies
}MOVertexArrayData;

typedef struct
{
	MetaObjectHeader	objectHeader;
	MOVertexArrayData	objectData;
}MOVertexArrayObject;



		/*****************/
		/* MATRIX OBJECT */
		/*****************/

typedef struct
{
	MetaObjectHeader	objectHeader;
	OGLMatrix4x4		matrix;
}MOMatrixObject;


		/******************/
		/* PICTURE OBJECT */
		/******************/

#define PICTURE_FULL_SCREEN_SIZE_X	640				// use this as scaling reference base
#define PICTURE_FULL_SCREEN_SIZE_Y	480


typedef struct
{
	OGLPoint3D			drawCoord;
	float				drawScaleX,drawScaleY;
	int					fullWidth,fullHeight;
	int					numCellsWide, numCellsHigh;		// # cells w/h
	int					cellWidth,cellHeight;			// cell w/h
	MOMaterialObject	**materials;					// array of materials for each cell
}MOPictureData;

typedef struct
{
	MetaObjectHeader	objectHeader;
	MOPictureData		objectData;
}MOPictureObject;


		/*****************/
		/* SPRITE OBJECT */
		/*****************/

typedef struct
{
	float				width,height;			// pixel w/h of texture
	float				aspectRatio;			// h/w
	float				scaleBasis;

	OGLPoint3D			coord;
	float				scaleX,scaleY;
	float				rot;

	MOMaterialObject	*material;
}MOSpriteData;

typedef struct
{
	MetaObjectHeader	objectHeader;
	MOSpriteData		objectData;
}MOSpriteObject;


typedef struct
{
	Boolean	loadFile;				// true if want to create sprite from pict right now, otherwise use from gSpriteList

	FSSpec	spec;					// picture file to load as sprite
	GLint	pixelFormat;			// format to store loaded sprite as

	short	group,type;				// group and type of gSpriteList sprite to use
}MOSpriteSetupData;



//-----------------------------

void MO_InitHandler(void);
MetaObjectPtr MO_CreateNewObjectOfType(u_long type, u_long subType, void *data);
MetaObjectPtr MO_GetNewReference(MetaObjectPtr mo);
void MO_AppendToGroup(MOGroupObject *group, MetaObjectPtr newObject);
void MO_AttachToGroupStart(MOGroupObject *group, MetaObjectPtr newObject);
void MO_DrawGeometry_VertexArray(const MOVertexArrayData *data, const OGLSetupOutputType *setupInfo);
void MO_DrawGroup(const MOGroupObject *object, const OGLSetupOutputType *setupInfo);
void MO_DrawObject(const MetaObjectPtr object, const OGLSetupOutputType *setupInfo);
void MO_DrawMaterial(MOMaterialObject *matObj, const OGLSetupOutputType *setupInfo);
void MO_DrawMatrix(const MOMatrixObject *matObj, const OGLSetupOutputType *setupInfo);
void MO_DrawPicture(const MOPictureObject *picObj, const OGLSetupOutputType *setupInfo);
void MO_DisposeObjectReference(MetaObjectPtr obj);
void MO_DuplicateVertexArrayData(MOVertexArrayData *inData, MOVertexArrayData *outData);
void MO_DeleteObjectInfo_Geometry_VertexArray(MOVertexArrayData *data);
void MO_DisposeObject_Geometry_VertexArray(MOVertexArrayData *data);
void MO_CalcBoundingBox(MetaObjectPtr object, OGLBoundingBox *bBox);
MOMaterialObject *MO_GetTextureFromFile(FSSpec *spec, OGLSetupOutputType *setupInfo, int destPixelFormat);
void MO_SetPictureObjectCoordsToMouse(OGLSetupOutputType *info, MOPictureObject *obj);

void MO_DrawSprite(const MOSpriteObject *spriteObj, const OGLSetupOutputType *setupInfo);
void MO_VertexArray_OffsetUVs(MetaObjectPtr object, float du, float dv);
void MO_Object_OffsetUVs(MetaObjectPtr object, float du, float dv);



#endif