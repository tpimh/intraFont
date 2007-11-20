/*
 * intraFont.c
 * This file is used to display the PSP's internal font (pgf firmware files)
 * intraFont Version 0.2 by BenHur - http://www.psp-programming.com/benhur
 *
 * Uses parts of pgeFont by InsertWittyName - http://insomniac.0x89.org
 *
 * This work is licensed under the Creative Commons Attribution-Share Alike 3.0 License.
 * See LICENSE for more details.
 *
 */

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#include "intraFont.h"

static unsigned int __attribute__((aligned(16))) clut[16];


unsigned long intraFontGetV(unsigned long n, unsigned char *p, unsigned long *b) {
	unsigned long i,v=0;
	for(i=0;i<n;i++) {
	    v += ( ( p[(*b)/8] >> ((*b)%8) ) & 1) << i;
		(*b)++;
	}
	return v;
}

unsigned long* intraFontGetTable(FILE *file, unsigned long n_elements, unsigned long bp_element) {
	unsigned long len_table = ((n_elements*bp_element+31)/32)*4;
	unsigned char *raw_table = (unsigned char*)malloc(len_table*sizeof(unsigned char));
	if (raw_table == NULL) return NULL;
	if (fread(raw_table, len_table*sizeof(unsigned char), 1, file) != 1) {
		free(raw_table);
		return NULL;
	}
	unsigned long *table = (unsigned long*)malloc(n_elements*sizeof(unsigned long));
	if (table == NULL) {
		free(raw_table);
		return NULL;
	}
	unsigned long i,j=0;
	for (i=0;i<n_elements;i++) {
		table[i] = intraFontGetV(bp_element,raw_table,&j);
	}
	free(raw_table);
	return table;
}

unsigned char intraFontGetBMP(intraFont *font, unsigned short id, unsigned char glyphtype) {
	if (!font) return 0;
	if ((font->options & INTRAFONT_CACHE_ALL) == INTRAFONT_CACHE_ALL) return 0;
	
	Glyph *glyph;
	if (glyphtype & PGF_CHARGLYPH) {
		glyph = &(font->glyph[id]);
	} else {
		glyph = &(font->shadowGlyph[id]);
	}
	if (glyph->flags & PGF_CACHED) return 1;
	unsigned long b = glyph->ptr*8;
	
	if (glyph->width > 0 && glyph->height > 0) {
		if (!(glyph->flags & PGF_BMP_H_ROWS) != !(glyph->flags & PGF_BMP_V_ROWS)) { //H_ROWS xor V_ROWS
			if ((font->texX + glyph->width + 1) > font->texWidth) {
				font->texY += font->texYSize + 1;
				font->texX = 1;
			}
			if ((font->texY + glyph->height + 1) > font->texHeight) {
				font->texY = 1;
				font->texX = 1;
			}
			glyph->x=font->texX;
			glyph->y=font->texY;
            
			/*draw bmp*/
			int i=0,j,xx,yy;
			unsigned char nibble, value = 0;
			while (i<(glyph->width*glyph->height)) {
				nibble = intraFontGetV(4,font->fontdata,&b);
				if (nibble < 8) value = intraFontGetV(4,font->fontdata,&b);
				for (j=0; (j<=((nibble<8)?(nibble):(15-nibble))) && (i<(glyph->width*glyph->height)); j++) {
					if (nibble >= 8) value = intraFontGetV(4,font->fontdata,&b);
					if (glyph->flags & PGF_BMP_H_ROWS) {
						xx = i%glyph->width;
						yy = i/glyph->width;
					} else {
						xx = i/glyph->height;
						yy = i%glyph->height;
					}
					if ((font->texX + xx) & 1) {
						font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] &= 0x0F;
						font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] |= (value<<4);
					} else {
						font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] &= 0xF0;
						font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] |= (value);
					}
					i++;
				}				
			}
            //erase border around glyph
            for (i = font->texX/2; i < (font->texX+glyph->width+1)/2; i++) {
                font->texture[i + (font->texY-1)*font->texWidth/2] = 0;
                font->texture[i + (font->texY+glyph->height)*font->texWidth/2] = 0;
            }
            for (i = font->texY-1; i < (font->texY+glyph->height+1); i++) {
                font->texture[((font->texX-1) + (i*font->texWidth))>>1] &= (font->texX & 1) ? 0xF0 : 0x0F;
                font->texture[((font->texX+glyph->width) + (i*font->texWidth))>>1] &= ((font->texX+glyph->width) & 1) ? 0x0F : 0xF0;
            }
			font->texX += glyph->width+1; //add empty gap to prevent interpolation artifacts from showing
			
			//mark dirty glyphs as uncached
			for (i = 0; i < font->n_chars; i++) {
				if ( (font->glyph[i].flags & PGF_CACHED) && (font->glyph[i].y == glyph->y) ) {
					if ( (font->glyph[i].x+font->glyph[i].width+1) > glyph->x && font->glyph[i].x < (glyph->x+glyph->width+1) ) {
						font->glyph[i].flags -= PGF_CACHED;
					}
				}
			}
			for (i = 0; i < font->n_shadows; i++) {
				if ( (font->shadowGlyph[i].flags & PGF_CACHED) && (font->shadowGlyph[i].y == glyph->y) ) {
					if ( (font->shadowGlyph[i].x+font->shadowGlyph[i].width+1) > glyph->x && font->shadowGlyph[i].x < (glyph->x+glyph->width+1) ) {
						font->shadowGlyph[i].flags -= PGF_CACHED;
					}
				}
			}
		} else return 0; //transposition=0 or overlay glyph
	} else {
		glyph->x=0;
		glyph->y=0;
	}
	glyph->flags |= PGF_CACHED;
	return 1;
}

unsigned short intraFontGetGlyph(unsigned char *data, unsigned long *b, unsigned char glyphtype, signed long *advancemap, Glyph *glyph) {
    if (glyphtype & PGF_CHARGLYPH) {
        (*b) += 14; //skip offset pos value of shadow
    } else {
        (*b) += intraFontGetV(14,data,b)*8+14; //skip to shadow 
    }
    glyph->width=intraFontGetV(7,data,b);
	glyph->height=intraFontGetV(7,data,b);
	glyph->left=intraFontGetV(7,data,b);
	if (glyph->left >= 64) glyph->left -= 128;
	glyph->top=intraFontGetV(7,data,b);
	if (glyph->top >= 64) glyph->top -= 128;
    glyph->flags = intraFontGetV(6,data,b);
    if (glyph->flags & PGF_CHARGLYPH) {
        (*b) += 7; //skip magic number
		glyph->shadowID = intraFontGetV(9,data,b);
		(*b) += 24 + ((glyph->flags & PGF_NO_EXTRA1)?0:56) + ((glyph->flags & PGF_NO_EXTRA2)?0:56) + ((glyph->flags & PGF_NO_EXTRA3)?0:56); //skip to 6bits before end of header
		glyph->advance = advancemap[intraFontGetV(8,data,b)*2]/16;
    } else {
		glyph->shadowID = 65535;
        glyph->advance = 0;
    }    
	glyph->ptr = (*b)/8;
    return 1;
}

unsigned short intraFontGetID(intraFont* font, unsigned short ucs) {
	unsigned short j, id = 0;
	char found = 0;
	for (j = 0; j < font->charmap_compr_len && !found; j++) {
		if ((ucs >= font->charmap_compr[j*2]) && (ucs < (font->charmap_compr[j*2]+font->charmap_compr[j*2+1]))) {
			id += ucs - font->charmap_compr[j*2];
			found = 1;
		} else {
			id += font->charmap_compr[j*2+1];
		}
	}
	if (!found) return 65535;	//char not in charmap
	id = font->charmap[id];
	if (id >= font->n_chars) return 65535; //char not in fontdata
	//if (font->glyph[id].width == 0 || font->glyph[id].height == 0) return 65535; //char has no valid glyph
	return id;
}

static int intraFontSwizzle(intraFont *font) {
	int height = font->texHeight;
	int byteWidth = font->texWidth>>1;
	int textureSize = byteWidth*height;

	int rowBlocks = (byteWidth>>4);
	int rowBlocksAdd = (rowBlocks - 1)<<7;
	unsigned int blockAddress = 0;
	unsigned int *src = (unsigned int*) font->texture;
	static unsigned char *tData;

	tData = (unsigned char*) memalign(16,textureSize);
	if(!tData) return 0;

	int i,j;
	for(j = 0; j < height; j++, blockAddress += 16) {
		unsigned int *block = ((unsigned int*)&tData[blockAddress]);
		for(i=0; i < rowBlocks; i++) {
			*block++ = *src++;
			*block++ = *src++;
			*block++ = *src++;
			*block++ = *src++;
			block += 28;
		}
		if((j & 0x7) == 0x7) blockAddress += rowBlocksAdd;
	}

	free(font->texture);
	font->texture = tData;
	font->options |= INTRAFONT_CACHE_ALL;

	return 1;
}

int intraFontPreCache(intraFont *font) {
	if (!font) return 0;
	if ((font->options & INTRAFONT_CACHE_ALL) == INTRAFONT_CACHE_ALL) return 0; //already prechached?
	
    //cache all glyphs
	int i,y;
	font->texX = 1;
	font->texY = 1;
	font->texYSize = 0;
	for (i = 0; i < font->n_chars; i++) {
		y = font->texY;
		intraFontGetBMP(font,i,PGF_CHARGLYPH);		
		if (font->texY > y || font->texYSize < font->glyph[i].height) font->texYSize = font->glyph[i].height; //minimize ysize after newline in cache(only valid for precached glyphs)
		if (font->texY < y) return 0;                               //char did not fit into cache -> abort precache (should reset cache and glyph.flags)
	}
	for (i=0;i<font->n_shadows;i++) {
		y = font->texY;
		intraFontGetBMP(font,i,PGF_SHADOWGLYPH);
		//if (font->texX > 0) font->texX--;                           //no gap needed between shadows
		if (font->texY > y || font->texYSize < font->shadowGlyph[i].height) font->texYSize = font->shadowGlyph[i].height; //minimize ysize
		if (font->texY < y) return 0;                               //char did not fit into cache -> abort precache (should reset cache and glyph.flags)
	}
	font->texHeight = ((font->texY) + (font->texYSize) + 7)&~7;     
	if (font->texHeight > font->texWidth) font->texHeight = font->texWidth;
	//font->texture = (unsigned char*)realloc(font->texture, (font->texWidth*font->texHeight)>>1); //not needed before swizzle
	
	//reduce fontdata
	int index = 0, j;
	for (i = 0; i < font->n_chars; i++) {
		if ((font->glyph[i].flags & PGF_BMP_H_ROWS) && (font->glyph[i].flags & PGF_BMP_V_ROWS)) {
			for (j = 0; j < 6; j++, index++) {
				font->fontdata[index] = font->fontdata[(font->glyph[i].ptr)+j];
			}
			font->glyph[i].ptr = index - j;
		}
	}
	if (index == 0)	{
		free(font->fontdata); 
		font->fontdata = NULL;
	} else {
		unsigned char* newfontdata = (unsigned char*)malloc(index*sizeof(unsigned char));
		if (newfontdata) {
			memcpy(newfontdata, font->fontdata, index);
			free(font->fontdata);
			font->fontdata = newfontdata;
		}
		//font->fontdata = (unsigned char*)realloc(font->fontdata,index*sizeof(unsigned char));		
	}

	//swizzle texture
	sceKernelDcacheWritebackAll();
	intraFontSwizzle(font);
	sceKernelDcacheWritebackAll();
	
	return 1;
}

intraFont* intraFontLoad(const char *filename, unsigned short options) {
    unsigned long i,j;
	
    //open pgf file and get file size
    FILE *file = fopen(filename, "rb"); /* read from the file in binary mode */
	if ( file == NULL ) {
		return NULL;
	}
	fseek(file, 0, SEEK_END);
    unsigned long filesize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	//read pgf header
	PGF_Header header;
	if (fread(&header, sizeof(PGF_Header), 1, file) != 1) {
		fclose(file);
		return NULL;
	}
	
	//pgf header tests: valid and known pgf file?
	if ((strncmp(header.pgf_id,"PGF0",4) != 0) || (header.version != 6) || (header.revision != 2 && header.revision != 3) ||
		(header.charmap_len > 65535) || (header.charptr_len > 65535) || (header.charmap_bpe > 32) || (header.charptr_bpe > 32) ||
		(header.charmap_min > header.charmap_max) || (header.shadowmap_len > 511) || (header.shadowmap_bpe > 16)) {
	    fclose(file);
		return NULL; 
	}
	
	//create and initialize font structure
	intraFont* font = (intraFont*)malloc(sizeof(intraFont));
	if (font == NULL) {
		fclose(file);
		return NULL;
	}	
	
	font->n_chars = (unsigned short)header.charptr_len;
	font->charmap_compr_len = (header.revision == 3)?7:1;
	font->texWidth = (options & INTRAFONT_CACHE_LARGE) ? (512) : ( (options & INTRAFONT_CACHE_SMALL) ? (128) : (256) );
	font->texHeight = font->texWidth;
	font->texX = 1;
	font->texY = 1;
	font->texYSize = 0;	
	font->advancex = header.fixedsize[0]/16;
	font->advancey = header.fixedsize[1]/16;
	font->n_shadows = (unsigned short)header.shadowmap_len;
	font->shadowscale = (unsigned char)header.shadowscale[0];
    font->options = options; 
	if ((options & INTRAFONT_CACHE_LARGE) && (options & INTRAFONT_CACHE_SMALL)) font->options -= INTRAFONT_CACHE_SMALL; //both are set -> use large texture
    font->size = 1.0f;               //default size
    font->color = 0xFFFFFFFF;        //non-transparent white
    font->shadowColor = 0xFF000000;  //non-transparent black
	
	font->filename = (char*)malloc((strlen(filename)+1)*sizeof(char));
	font->glyph = (Glyph*)malloc(font->n_chars*sizeof(Glyph));
	font->shadowGlyph = (Glyph*)malloc(font->n_shadows*sizeof(Glyph));
	font->charmap_compr = (unsigned short*)malloc(font->charmap_compr_len*sizeof(unsigned short)*2);
	font->charmap = (unsigned short*)malloc(header.charmap_len*sizeof(unsigned short));
	font->texture = (unsigned char*)memalign(16,font->texWidth*font->texHeight>>1);

	if (!font->filename || !font->glyph || !font->shadowGlyph || !font->charmap_compr || !font->charmap || !font->texture) {
		fclose(file);
		intraFontUnload(font);
		return NULL;
	}	
	strcpy(font->filename,filename);
	memset(font->glyph, 0, font->n_chars*sizeof(Glyph));
	memset(font->shadowGlyph, 0, font->n_shadows*sizeof(Glyph));
	memset(font->texture, 0, font->texWidth*font->texHeight>>1);
	
	//read advance table
    fseek(file, header.header_len+(header.table1_len+header.table2_len+header.table3_len)*8, SEEK_SET);
	signed long *advancemap = (signed long*)malloc(header.advance_len*sizeof(signed long)*2);
	if (!advancemap) {
		fclose(file);
		intraFontUnload(font);
		return NULL;
	}	
	if (fread(advancemap, header.advance_len*sizeof(signed long)*2, 1, file) != 1) {
		free(advancemap);
		fclose(file);
		intraFontUnload(font);
		return NULL;
	}	
	
	//read shadowmap
	unsigned long *ucs_shadowmap = intraFontGetTable(file, header.shadowmap_len, header.shadowmap_bpe);
	if (ucs_shadowmap == NULL) {
		free(advancemap);	
		fclose(file);
		intraFontUnload(font);
		return NULL;
	}
	
	//version 6.3 charmap compression
	if (header.revision == 3) {
		if (fread(font->charmap_compr, font->charmap_compr_len*sizeof(unsigned short)*2, 1, file) != 1) {
			free(advancemap);		
		    free(ucs_shadowmap);
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}
	} else {
		font->charmap_compr[0] = header.charmap_min;
		font->charmap_compr[1] = header.charmap_len;
	}
	
	//read charmap
	unsigned long *id_charmap = intraFontGetTable(file, header.charmap_len, header.charmap_bpe);
	if (id_charmap == NULL) {
		free(advancemap);	
	    free(ucs_shadowmap);
		fclose(file);
		intraFontUnload(font);
		return NULL;
	}
	for (i=0;i<header.charmap_len;i++) {
		font->charmap[i]=(unsigned short)((id_charmap[i] < font->n_chars)?id_charmap[i]:65535);
	}
	free(id_charmap);
	
	//read charptr
	unsigned long *charptr = intraFontGetTable(file, header.charptr_len, header.charptr_bpe);
	if (charptr == NULL) {
		free(advancemap);	
	    free(ucs_shadowmap);
		fclose(file);
		intraFontUnload(font);
		return NULL;
	}
	
	//read raw fontdata
	unsigned long start_fontdata = ftell(file);
	unsigned long len_fontdata = filesize-start_fontdata;
	font->fontdata = (unsigned char*)malloc(len_fontdata*sizeof(unsigned char));
	if (font->fontdata == NULL) {
		free(advancemap);	
	    free(ucs_shadowmap);
		free(charptr);
		fclose(file);
		intraFontUnload(font);
		return NULL;
	}
	if (fread(font->fontdata, len_fontdata*sizeof(unsigned char), 1, file) != 1) {
		free(advancemap);	
		free(ucs_shadowmap);
		free(charptr);
		fclose(file);
		intraFontUnload(font);
		return NULL;
	}
	
	//close file	
	fclose(file);
	
	//populate chars and count space used in cache to prebuffer all chars
	int x=1,y=1,ysize=0;
	for (i=0;i<font->n_chars;i++) {
		j = charptr[i]*4*8;
		if (!intraFontGetGlyph(font->fontdata, &j, PGF_CHARGLYPH, advancemap, &(font->glyph[i]))) {
			free(advancemap);			
			free(ucs_shadowmap);
			free(charptr);
			intraFontUnload(font);
			return NULL;
		}
		if (!(font->glyph[i].flags & PGF_BMP_H_ROWS) != !(font->glyph[i].flags & PGF_BMP_V_ROWS)) { //H_ROWS xor V_ROWS (real glyph, not overlay)
			if (font->glyph[i].height > font->texYSize) font->texYSize = font->glyph[i].height;     //find max glyph height			
			if ((x + font->glyph[i].width) > font->texWidth) {                             
				y += ysize+1;
				x = 1;
				ysize = 0;
			} 
			if (font->glyph[i].height > ysize) ysize = font->glyph[i].height;
			x += font->glyph[i].width+1;
		}		
	} 
	
	//populate shadows and count space
	unsigned short char_id,shadow_id;
	for (i = 0; i<font->n_shadows; i++) {
		char_id = intraFontGetID(font,ucs_shadowmap[i]);
		if (char_id < font->n_chars) {
			j = charptr[char_id]*4*8;
			shadow_id = font->glyph[char_id].shadowID;
			if (!intraFontGetGlyph(font->fontdata, &j, PGF_SHADOWGLYPH, NULL, &(font->shadowGlyph[shadow_id]))) {
				free(advancemap);			
				free(ucs_shadowmap);
				free(charptr);
				intraFontUnload(font);
				return NULL;
			}			
			if (!(font->shadowGlyph[shadow_id].flags & PGF_BMP_H_ROWS) != !(font->shadowGlyph[shadow_id].flags & PGF_BMP_V_ROWS)) { //H_ROWS xor V_ROWS (real glyph, not overlay)
				if (font->shadowGlyph[shadow_id].height > font->texYSize) font->texYSize = font->shadowGlyph[shadow_id].height;     //find max glyph height
				if ((x + font->shadowGlyph[shadow_id].width) > font->texWidth) {                          
					y += ysize+1;
					x = 1;
					ysize = 0;
				} 
				if (font->shadowGlyph[shadow_id].height > ysize) ysize = font->shadowGlyph[shadow_id].height;
				x += font->shadowGlyph[shadow_id].width+1;
			}
		}		              
	}
	
	//free temp stuff
	free(advancemap);
	free(ucs_shadowmap);
	free(charptr);
	
	//cache chars, swizzle texture and free unneeded stuff (if INTRAFONT_CACHE_ALL and cache big enough)
	sceKernelDcacheWritebackAll();
	if ( ((options & INTRAFONT_CACHE_ALL) == INTRAFONT_CACHE_ALL) && ((y + ysize + 1) <= font->texHeight) ) { //cache all and does it fit into cache?
		if (!intraFontPreCache(font)) {
			intraFontUnload(font);
			return NULL;			
		}
	}
	sceKernelDcacheWritebackAll();

	return font;
}

void intraFontUnload(intraFont *font) {
	if (!font) return;
    if (font->filename) free(font->filename);
    if (font->fontdata) free(font->fontdata);
	if (font->texture) free(font->texture);
	if (font->charmap_compr) free(font->charmap_compr);
	if (font->charmap) free(font->charmap);
	if (font->glyph) free(font->glyph);
	if (font->shadowGlyph) free(font->shadowGlyph);
	if (font) free(font);
}

int intraFontInit(void) {
	int n;
	for(n = 0; n < 16; n++)
		clut[n] = ((n * 17) << 24) | 0xffffff;
	return 1;
}

void intraFontShutdown(void) {
	//Nothing yet
}

void intraFontActivate(intraFont *font) {
	if(!font) return;
	if(!font->texture) return;

	sceGuClutMode(GU_PSM_8888, 0, 255, 0);
	sceGuClutLoad(2, clut);

	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexMode(GU_PSM_T4, 0, 0, ((font->options & INTRAFONT_CACHE_ALL) == INTRAFONT_CACHE_ALL) ? 1 : 0);
	sceGuTexImage(0, font->texWidth, font->texWidth, font->texWidth, font->texture);
	sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
	sceGuTexEnvColor(0x0);
	sceGuTexOffset(0.0f, 0.0f);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
}

void intraFontSetStyle(intraFont *font, float size, unsigned int color, unsigned int shadowColor, unsigned short options) {
	if (!font) return;
	font->size = size;
	font->color = color;
	font->shadowColor = shadowColor;
	font->options = (options & (INTRAFONT_ADVANCE_V+INTRAFONT_ALIGN_CENTER+INTRAFONT_ALIGN_RIGHT+INTRAFONT_WIDTH_FIX+INTRAFONT_ACTIVE+PGF_WIDTH_MASK)) +
	                (font->options & INTRAFONT_CACHE_ALL);
	if ((font->options & PGF_WIDTH_MASK) == 0) font->options |= ((font->advancex / 8) & PGF_WIDTH_MASK);
}

float intraFontPrintf(intraFont *font, float x, float y, const char *text, ...) {
	if(!font) return x;

	char buffer[256];
	va_list ap;
	
	va_start(ap, text);
	vsnprintf(buffer, 256, text, ap);
	va_end(ap);
	buffer[255] = 0;
	
	return intraFontPrint(font, x, y, buffer);
}

float intraFontPrint(intraFont *font, float x, float y, const char *text) {
    if (!text || strlen(text) == 0 || !font) return x;
	
    unsigned short* ucs2_text = (unsigned short*)malloc((strlen(text)+1)*sizeof(unsigned short));
    if (!ucs2_text) return x;
	
    int i;
    for (i = 0; i < strlen(text); i++) ucs2_text[i] = text[i];
    ucs2_text[i] = 0;
	
    x = intraFontPrintUCS2(font, x, y, ucs2_text);
	
    free(ucs2_text);
	
    return x;
}

float intraFontPrintUCS2(intraFont *font, float x, float y, const unsigned short *text) {
	if(!font) return x;
	
	float glyphscale = font->size;
	float width = 0.0f, height = font->advancey * glyphscale / 4.0;
	float left = x, top = y - height; 
	if (font->options & INTRAFONT_ALIGN_RIGHT)  left -= intraFontMeasureTextUCS2(font, text);
	if (font->options & INTRAFONT_ALIGN_CENTER) left -= intraFontMeasureTextUCS2(font, text)/2.0;
	
	typedef struct {
		float u, v;
		unsigned int c;
		float x, y, z;
	} fontVertex;
	fontVertex *v, *v0, *v1, *s0, *s1;
	
	//count number of characters
	int length = 0;
	while (text[length] > 0) length++;
	if (length == 0) return x;
	
	//count number of glyphs to draw and cache BMPs
	int i, j, n_glyphs, last_n_glyphs, n_sglyphs, changed, reactivate = 0, count = 0;
	unsigned short char_id, subucs2, glyph_id;
	do {
		changed = 0;
		n_glyphs = 0;
		n_sglyphs = 0;
		last_n_glyphs = 0;
		for(i = 0; i < length; i++) {
		
			char_id = intraFontGetID(font,text[i]); //char
			if (char_id < font->n_chars) {
				if ((font->glyph[char_id].flags & PGF_BMP_OVERLAY) == PGF_BMP_OVERLAY) { //overlay glyph?
					for (j = 0; j < 3; j++) {
						subucs2 = font->fontdata[(font->glyph[char_id].ptr)+j*2] + font->fontdata[(font->glyph[char_id].ptr)+j*2+1] * 256;				
						if (subucs2) {
							glyph_id = intraFontGetID(font, subucs2);
							if (glyph_id < font->n_chars) {
								n_glyphs++;
								if (!(font->glyph[glyph_id].flags & PGF_CACHED)) {
									if (intraFontGetBMP(font,glyph_id,PGF_CHARGLYPH)) { changed = 1; reactivate = 1; }
								}								
							}
						}
					}
				} else {
					n_glyphs++; 
					if (!(font->glyph[char_id].flags & PGF_CACHED)) {
						if (intraFontGetBMP(font,char_id,PGF_CHARGLYPH)) { changed = 1; reactivate = 1; }
					}
				}
			
				if (n_glyphs > last_n_glyphs) {
					n_sglyphs++; //shadow
					if (!(font->shadowGlyph[font->glyph[char_id].shadowID].flags & PGF_CACHED)) {
						if (intraFontGetBMP(font,font->glyph[char_id].shadowID,PGF_SHADOWGLYPH)) { changed = 1; reactivate = 1; }
					}
					last_n_glyphs = n_glyphs;
				}
				
			}
			
		}
		count++;
	} while (changed && count <= length);
	if (changed) return 0.0f; //not all chars fit into texture -> abort (better solution: split up string and call intraFontPrintUCS2 twice)
	
	//reserve memory in displaylist
	v = sceGuGetMemory((sizeof(fontVertex)<<2) * (n_glyphs+n_sglyphs));

	int s_index = 0, c_index = n_sglyphs, last_c_index = n_sglyphs; // index for shadow and character/overlay glyphs	
	for(i = 0; i < length; i++) {
		unsigned short char_id = intraFontGetID(font,text[i]);
		if (char_id < font->n_chars) {
		
			//center glyphs for monospace
			if (font->options & INTRAFONT_WIDTH_FIX) {
				width += ( ((float)(font->options & PGF_WIDTH_MASK))/2.0f - ((float)font->glyph[char_id].advance)/8.0f ) * glyphscale ;
			}
		
			//add vertices for subglyhs				
			for (j = 0; j < 3; j++) {			
				if ((font->glyph[char_id].flags & PGF_BMP_OVERLAY) == PGF_BMP_OVERLAY) {
					subucs2 = font->fontdata[(font->glyph[char_id].ptr)+j*2] + font->fontdata[(font->glyph[char_id].ptr)+j*2+1] * 256;				
					glyph_id = intraFontGetID(font, subucs2);
				} else {
					glyph_id = char_id;
					j = 2;
				}
		
				if (glyph_id < font->n_chars) {
					Glyph *glyph = &(font->glyph[glyph_id]);

					v0 = &v[(c_index<<1) + 0];
					v1 = &v[(c_index<<1) + 1];

					v0->u = glyph->x;//+0.25f;
					v0->v = glyph->y;//+0.25f;
					v0->c = font->color;
					v0->x = left + width + glyph->left*glyphscale;
					v0->y = top + height - glyph->top *glyphscale;
					v0->z = 0.0f;

					v1->u = (glyph->x + glyph->width);//-0.25f;
					v1->v = (glyph->y + glyph->height);//-0.25f;
					v1->c = font->color;
					v1->x = left + width + (glyph->width+glyph->left)*glyphscale;
					v1->y = top + height + (glyph->height-glyph->top)*glyphscale;
					v1->z = 0.0f;
					
					c_index++;
				}
			}
				
			//add vertices for shadow
			if (c_index > last_c_index) {
				Glyph *shadowGlyph = &(font->shadowGlyph[font->glyph[char_id].shadowID]);
		
				s0 = &v[(s_index<<1) + 0];
				s1 = &v[(s_index<<1) + 1];

				s0->u = shadowGlyph->x;//+0.5f;
				s0->v = shadowGlyph->y;//+0.5f;
				s0->c = font->shadowColor;
				s0->x = left + width + shadowGlyph->left*glyphscale*64.0f/((float)font->shadowscale);
				s0->y = top + height - shadowGlyph->top *glyphscale*64.0f/((float)font->shadowscale);
				s0->z = 0.0f;

				s1->u = (shadowGlyph->x + shadowGlyph->width);//-0.5f;
				s1->v = (shadowGlyph->y + shadowGlyph->height);//-0.5f;
				s1->c = font->shadowColor;
				s1->x = left + width + (shadowGlyph->width+shadowGlyph->left)*glyphscale*64.0f/((float)font->shadowscale);
				s1->y = top + height + (shadowGlyph->height-shadowGlyph->top)*glyphscale*64.0f/((float)font->shadowscale);
				s1->z = 0.0f;
			
				s_index++;
				last_c_index = c_index;
			}

			// advance
			if (font->options & INTRAFONT_WIDTH_FIX) {
				width += ( ((float)(font->options & PGF_WIDTH_MASK))/2.0f + ((float)font->glyph[char_id].advance)/8.0f ) * glyphscale; 
			} else {
				width += font->glyph[char_id].advance * glyphscale * 0.25;
			}
			
		} 
		
		if (text[i] == '\n') {
			left = x;
			if (font->options & INTRAFONT_ALIGN_RIGHT)  left -= intraFontMeasureTextUCS2(font, text+i+1);
			if (font->options & INTRAFONT_ALIGN_CENTER) left -= intraFontMeasureTextUCS2(font, text+i+1)/2.0;
			width = 0.0f;
			height += font->advancey * glyphscale / 4.0;
		}
	}
		
	//finalize and activate texture (if not already active or has been changed)
	sceKernelDcacheWritebackAll();
	if (!(font->options & INTRAFONT_ACTIVE) || (reactivate == 1)) intraFontActivate(font);
	
	sceGuDisable(GU_DEPTH_TEST);
	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, (n_glyphs+n_sglyphs)<<2, 0, v);
	sceGuEnable(GU_DEPTH_TEST);
	
	return left+width;
}

float intraFontMeasureText(intraFont *font, const char *text) { 
    if (!font) return 0.0f; 
	
	int i, length = 0;
	float x = 0.0f;
	
	while ((text[length] > 0) && (text[length] != '\n')) length++; //strlen until end of string or newline
	
	for(i = 0; i < strlen(text); i++) { 
		unsigned short char_id = intraFontGetID(font,text[i]); 
		if (char_id < font->n_chars) 
			x += (font->options & INTRAFONT_WIDTH_FIX) ? (font->options & PGF_WIDTH_MASK)*font->size : (font->glyph[char_id].advance)*font->size*0.25f; 
	}    
	
    return x; 
} 

float intraFontMeasureTextUCS2(intraFont *font, const unsigned short *text) { 
   if(!font) return 0.0f; 

   int i, length = 0; 
   float x = 0.0f; 

   while ((text[length] > 0) && (text[length] != '\n')) length++; //strlen until end of string or newline

   for(i = 0; i < length; i++) { 
		unsigned short char_id = intraFontGetID(font,text[i]); 
		if (char_id < font->n_chars) 
			x += (font->options & INTRAFONT_WIDTH_FIX) ? (font->options & PGF_WIDTH_MASK)*font->size : (font->glyph[char_id].advance)*font->size*0.25f; 
	} 

   return x; 
}

