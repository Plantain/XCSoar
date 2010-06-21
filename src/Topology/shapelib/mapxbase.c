/*
** This code is entirely based on the previous work of Frank Warmerdam. It is
** essentially shapelib 1.1.5. However, there were enough changes that it was
** incorporated into the MapServer source to avoid confusion. See the README
** for licence details.
*/
/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "mapprimitive.h"
#include "mapshape.h"
#include "maperror.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/************************************************************************/
/*                             SfRealloc()                              */
/*                                                                      */
/*      A realloc cover function that will access a NULL pointer as     */
/*      a valid input.                                                  */
/************************************************************************/
static void *
SfRealloc(void *pMem, size_t nNewSize)
{
  return realloc(pMem, nNewSize);
}

/************************************************************************/
/*                           writeHeader()                              */
/*                                                                      */
/*      This is called to write out the file header, and field          */
/*      descriptions before writing any actual data records.  This      */
/*      also computes all the DBFDataSet field offset/size/decimals     */
/*      and so forth values.                                            */
/************************************************************************/
static void writeHeader(DBFHandle psDBF)

{
    uchar	abyHeader[32];
    int		i;

    if( !psDBF->bNoHeader )
        return;

    psDBF->bNoHeader = MS_FALSE;

    /* -------------------------------------------------------------------- */
    /*	Initialize the file header information.		    		    */
    /* -------------------------------------------------------------------- */
    for( i = 0; i < 32; i++ )
        abyHeader[i] = 0;

    abyHeader[0] = 0x03;		/* memo field? - just copying 	*/

    /* date updated on close, record count preset at zero */

    abyHeader[8] = psDBF->nHeaderLength % 256;
    abyHeader[9] = psDBF->nHeaderLength / 256;

    abyHeader[10] = psDBF->nRecordLength % 256;
    abyHeader[11] = psDBF->nRecordLength / 256;

    /* -------------------------------------------------------------------- */
    /*      Write the initial 32 byte file header, and all the field        */
    /*      descriptions.                                     		    */
    /* -------------------------------------------------------------------- */
    fseek( psDBF->fp, 0, 0 );
    fwrite( abyHeader, 32, 1, psDBF->fp );
    fwrite( psDBF->pszHeader, 32, psDBF->nFields, psDBF->fp );

    /* -------------------------------------------------------------------- */
    /*      Write out the newline character if there is room for it.        */
    /* -------------------------------------------------------------------- */
    if( psDBF->nHeaderLength > 32*psDBF->nFields + 32 )
    {
        char	cNewline;

        cNewline = 0x0d;
        fwrite( &cNewline, 1, 1, psDBF->fp );
    }
}

/************************************************************************/
/*                           flushRecord()                              */
/*                                                                      */
/*      Write out the current record if there is one.                   */
/************************************************************************/
static void flushRecord( DBFHandle psDBF )

{
    int		nRecordOffset;

    if( psDBF->bCurrentRecordModified && psDBF->nCurrentRecord > -1 )
    {
	psDBF->bCurrentRecordModified = MS_FALSE;

	nRecordOffset = psDBF->nRecordLength * psDBF->nCurrentRecord
	                                             + psDBF->nHeaderLength;

	fseek( psDBF->fp, nRecordOffset, 0 );
	fwrite( psDBF->pszCurrentRecord, psDBF->nRecordLength, 1, psDBF->fp );
    }
}

/************************************************************************/
/*                              msDBFOpen()                             */
/*                                                                      */
/*      Open a .dbf file.                                               */
/************************************************************************/
DBFHandle msDBFOpen( const char * pszFilename, const char * pszAccess )

{
    DBFHandle		psDBF;
    uchar		*pabyBuf;
    int			nFields, nRecords, nHeadLen, nRecLen, iField;
    char	        *pszDBFFilename;

    /* -------------------------------------------------------------------- */
    /*      We only allow the access strings "rb" and "r+".                 */
    /* -------------------------------------------------------------------- */
    if( strcmp(pszAccess,"r") != 0 && strcmp(pszAccess,"r+") != 0
        && strcmp(pszAccess,"rb") != 0 && strcmp(pszAccess,"r+b") != 0 )
        return( NULL );

    /* -------------------------------------------------------------------- */
    /*	Ensure the extension is converted to dbf or DBF if it is 	    */
    /*	currently .shp or .shx.						    */
    /* -------------------------------------------------------------------- */
    pszDBFFilename = (char *) malloc(strlen(pszFilename)+1);
    strcpy( pszDBFFilename, pszFilename );

    if( strcmp(pszFilename+strlen(pszFilename)-4,".shp")
	|| strcmp(pszFilename+strlen(pszFilename)-4,".shx") )
    {
        strcpy( pszDBFFilename+strlen(pszDBFFilename)-4, ".dbf");
    }
    else if( strcmp(pszFilename+strlen(pszFilename)-4,".SHP")
	     || strcmp(pszFilename+strlen(pszFilename)-4,".SHX") )
    {
        strcpy( pszDBFFilename+strlen(pszDBFFilename)-4, ".DBF");
    }

    /* -------------------------------------------------------------------- */
    /*      Open the file.                                                  */
    /* -------------------------------------------------------------------- */
    psDBF = (DBFHandle) calloc( 1, sizeof(DBFInfo) );
    psDBF->zfp = ppc_fopen( pszDBFFilename, pszAccess );
    if( psDBF->zfp == NULL )
        return( NULL );

    psDBF->bNoHeader = MS_FALSE;
    psDBF->nCurrentRecord = -1;
    psDBF->bCurrentRecordModified = MS_FALSE;

    psDBF->pszStringField = NULL;
    psDBF->nStringFieldLen = 0;

    free( pszDBFFilename );

    /* -------------------------------------------------------------------- */
    /*  Read Table Header info                                              */
    /* -------------------------------------------------------------------- */
    pabyBuf = (uchar *) malloc(500);
    zzip_fread( pabyBuf, 32, 1, psDBF->zfp );

    psDBF->nRecords = nRecords =
     pabyBuf[4] + pabyBuf[5]*256 + pabyBuf[6]*256*256 + pabyBuf[7]*256*256*256;

    psDBF->nHeaderLength = nHeadLen = pabyBuf[8] + pabyBuf[9]*256;
    psDBF->nRecordLength = nRecLen = pabyBuf[10] + pabyBuf[11]*256;

    psDBF->nFields = nFields = (nHeadLen - 32) / 32;

    psDBF->pszCurrentRecord = (char *) malloc(nRecLen);

    /* -------------------------------------------------------------------- */
    /*  Read in Field Definitions                                           */
    /* -------------------------------------------------------------------- */
    pabyBuf = (uchar *) SfRealloc(pabyBuf,nHeadLen);
    psDBF->pszHeader = (char *) pabyBuf;

    zzip_seek( psDBF->zfp, 32, 0 );
    zzip_fread( pabyBuf, nHeadLen, 1, psDBF->zfp );

    psDBF->panFieldOffset = (int *) malloc(sizeof(int) * nFields);
    psDBF->panFieldSize = (int *) malloc(sizeof(int) * nFields);
    psDBF->panFieldDecimals = (int *) malloc(sizeof(int) * nFields);
    psDBF->pachFieldType = (char *) malloc(sizeof(char) * nFields);

    for( iField = 0; iField < nFields; iField++ )
    {
	uchar		*pabyFInfo;

	pabyFInfo = pabyBuf+iField*32;

	if( pabyFInfo[11] == 'N' || pabyFInfo[11] == 'F' )
	{
	    psDBF->panFieldSize[iField] = pabyFInfo[16];
	    psDBF->panFieldDecimals[iField] = pabyFInfo[17];
	}
	else
	{
	    psDBF->panFieldSize[iField] = pabyFInfo[16] + pabyFInfo[17]*256;
	    psDBF->panFieldDecimals[iField] = 0;
	}

	psDBF->pachFieldType[iField] = (char) pabyFInfo[11];
	if( iField == 0 )
	    psDBF->panFieldOffset[iField] = 1;
	else
	    psDBF->panFieldOffset[iField] =
	      psDBF->panFieldOffset[iField-1] + psDBF->panFieldSize[iField-1];
    }

    return( psDBF );
}

/************************************************************************/
/*                              msDBFClose()                            */
/************************************************************************/

void  msDBFClose(DBFHandle psDBF)
{
  /* -------------------------------------------------------------------- */
  /*      Write out header if not already written.                        */
  /* -------------------------------------------------------------------- */
    if( psDBF->bNoHeader )
        writeHeader( psDBF );

    flushRecord( psDBF );

    /* -------------------------------------------------------------------- */
    /*      Update last access date, and number of records if we have       */
    /*	write access.                					    */
    /* -------------------------------------------------------------------- */
    if( psDBF->bUpdated && psDBF->fp)
    {
	uchar		abyFileHeader[32];

	fseek( psDBF->fp, 0, 0 );
	fread( abyFileHeader, 32, 1, psDBF->fp );

	abyFileHeader[1] = 95;			/* YY */
	abyFileHeader[2] = 7;			/* MM */
	abyFileHeader[3] = 26;			/* DD */

	abyFileHeader[4] = psDBF->nRecords % 256;
	abyFileHeader[5] = (psDBF->nRecords/256) % 256;
	abyFileHeader[6] = (psDBF->nRecords/(256*256)) % 256;
	abyFileHeader[7] = (psDBF->nRecords/(256*256*256)) % 256;

	fseek( psDBF->fp, 0, 0 );
	fwrite( abyFileHeader, 32, 1, psDBF->fp );
    }

    /* -------------------------------------------------------------------- */
    /*      Close, and free resources.                                      */
    /* -------------------------------------------------------------------- */
    if (psDBF->fp) {
      fclose( psDBF->fp );
      psDBF->fp = 0;
    }
    if (psDBF->zfp) {
      zzip_fclose( psDBF->zfp );
      psDBF->zfp = 0;
    }

    if( psDBF->panFieldOffset != NULL )
    {
        free( psDBF->panFieldOffset );
        free( psDBF->panFieldSize );
        free( psDBF->panFieldDecimals );
        free( psDBF->pachFieldType );
    }

    free( psDBF->pszHeader );
    free( psDBF->pszCurrentRecord );

    if(psDBF->pszStringField) free(psDBF->pszStringField);

    free( psDBF );
}

/************************************************************************/
/*                             msDBFCreate()                            */
/*                                                                      */
/*      Create a new .dbf file.                                         */
/************************************************************************/
DBFHandle msDBFCreate( const char * pszFilename )

{
    DBFHandle	psDBF;
    FILE	*fp;

    /* -------------------------------------------------------------------- */
    /*      Create the file.                                                */
    /* -------------------------------------------------------------------- */
    fp = fopen( pszFilename, "wb" );
    if( fp == NULL )
        return( NULL );

    fputc( 0, fp );
    fclose( fp );

    fp = fopen( pszFilename, "rb+" );
    if( fp == NULL )
        return( NULL );

    /* -------------------------------------------------------------------- */
    /*	Create the info structure.			  		    */
    /* -------------------------------------------------------------------- */
    psDBF = (DBFHandle) malloc(sizeof(DBFInfo));

    psDBF->fp = fp;
    psDBF->zfp = NULL;
    psDBF->nRecords = 0;
    psDBF->nFields = 0;
    psDBF->nRecordLength = 1;
    psDBF->nHeaderLength = 33;

    psDBF->panFieldOffset = NULL;
    psDBF->panFieldSize = NULL;
    psDBF->panFieldDecimals = NULL;
    psDBF->pachFieldType = NULL;
    psDBF->pszHeader = NULL;

    psDBF->nCurrentRecord = -1;
    psDBF->bCurrentRecordModified = MS_FALSE;
    psDBF->pszCurrentRecord = NULL;


    psDBF->pszStringField = NULL;

    psDBF->nStringFieldLen = 0;


    psDBF->bNoHeader = MS_TRUE;
    psDBF->bUpdated = MS_FALSE;

    return( psDBF );
}

/************************************************************************/
/*                            msDBFAddField()                           */
/*                                                                      */
/*      Add a field to a newly created .dbf file before any records     */
/*      are written.                                                    */
/************************************************************************/
int	msDBFAddField(DBFHandle psDBF, const char * pszFieldName, DBFFieldType eType, int nWidth, int nDecimals )
{
    char	*pszFInfo;
    int		i;

    /* -------------------------------------------------------------------- */
    /*      Do some checking to ensure we can add records to this file.     */
    /* -------------------------------------------------------------------- */
    if( psDBF->nRecords > 0 )
        return( MS_FALSE );

    if( !psDBF->bNoHeader )
        return( MS_FALSE );

    if( eType != FTDouble && nDecimals != 0 )
        return( MS_FALSE );

    /* -------------------------------------------------------------------- */
    /*      SfRealloc all the arrays larger to hold the additional field    */
    /*      information.                                                    */
    /* -------------------------------------------------------------------- */
    psDBF->nFields++;

    psDBF->panFieldOffset = (int *)
      SfRealloc( psDBF->panFieldOffset, sizeof(int) * psDBF->nFields );

    psDBF->panFieldSize = (int *)
      SfRealloc( psDBF->panFieldSize, sizeof(int) * psDBF->nFields );

    psDBF->panFieldDecimals = (int *)
      SfRealloc( psDBF->panFieldDecimals, sizeof(int) * psDBF->nFields );

    psDBF->pachFieldType = (char *)
      SfRealloc( psDBF->pachFieldType, sizeof(char) * psDBF->nFields );

    /* -------------------------------------------------------------------- */
    /*      Assign the new field information fields.                        */
    /* -------------------------------------------------------------------- */
    psDBF->panFieldOffset[psDBF->nFields-1] = psDBF->nRecordLength;
    psDBF->nRecordLength += nWidth;
    psDBF->panFieldSize[psDBF->nFields-1] = nWidth;
    psDBF->panFieldDecimals[psDBF->nFields-1] = nDecimals;

    if( eType == FTString )
        psDBF->pachFieldType[psDBF->nFields-1] = 'C';
    else
        psDBF->pachFieldType[psDBF->nFields-1] = 'N';

    /* -------------------------------------------------------------------- */
    /*      Extend the required header information.                         */
    /* -------------------------------------------------------------------- */
    psDBF->nHeaderLength += 32;
    psDBF->bUpdated = MS_FALSE;

    psDBF->pszHeader = (char *) SfRealloc(psDBF->pszHeader,psDBF->nFields*32);

    pszFInfo = psDBF->pszHeader + 32 * (psDBF->nFields-1);

    for( i = 0; i < 32; i++ )
        pszFInfo[i] = '\0';

    if( strlen(pszFieldName) < 10 )
        strncpy( pszFInfo, pszFieldName, strlen(pszFieldName));
    else
        strncpy( pszFInfo, pszFieldName, 10);

    pszFInfo[11] = psDBF->pachFieldType[psDBF->nFields-1];

    if( eType == FTString )
    {
        pszFInfo[16] = nWidth % 256;
        pszFInfo[17] = nWidth / 256;
    }
    else
    {
        pszFInfo[16] = nWidth;
        pszFInfo[17] = nDecimals;
    }

    /* -------------------------------------------------------------------- */
    /*      Make the current record buffer appropriately larger.            */
    /* -------------------------------------------------------------------- */
    psDBF->pszCurrentRecord = (char *) SfRealloc(psDBF->pszCurrentRecord,
					       psDBF->nRecordLength);

    return( psDBF->nFields-1 );
}

/************************************************************************/
/*                          msDBFReadAttribute()                        */
/*                                                                      */
/*      Read one of the attribute fields of a record.                   */
/************************************************************************/
static char *msDBFReadAttribute(DBFHandle psDBF, int hEntity, int iField )

{
    int	       	nRecordOffset, i;
    const char *pabyRec;
    char	*pReturnField = NULL;

    /* -------------------------------------------------------------------- */
    /*	Is the request valid?                  				    */
    /* -------------------------------------------------------------------- */
    if( iField < 0 || iField >= psDBF->nFields )
    {
        msSetError(MS_DBFERR, "Invalid field index %d.", "msDBFGetItemIndex()",iField );
        return( NULL );
    }

    if( hEntity < 0 || hEntity >= psDBF->nRecords )
    {
        msSetError(MS_DBFERR, "Invalid record number %d.", "msDBFGetItemIndex()",hEntity );
        return( NULL );
    }

    /* -------------------------------------------------------------------- */
    /*	Have we read the record?					    */
    /* -------------------------------------------------------------------- */
    if( psDBF->nCurrentRecord != hEntity )
    {
	flushRecord( psDBF );

	nRecordOffset = psDBF->nRecordLength * hEntity + psDBF->nHeaderLength;

	zzip_seek( psDBF->zfp, nRecordOffset, 0 );
	zzip_fread( psDBF->pszCurrentRecord, psDBF->nRecordLength, 1, psDBF->zfp );

	psDBF->nCurrentRecord = hEntity;
    }

    pabyRec = (const char *) psDBF->pszCurrentRecord;

    /* -------------------------------------------------------------------- */
    /*	Ensure our field buffer is large enough to hold this buffer.	    */
    /* -------------------------------------------------------------------- */
    if( psDBF->panFieldSize[iField]+1 > psDBF->nStringFieldLen )
    {
	psDBF->nStringFieldLen = psDBF->panFieldSize[iField]*2 + 10;
	psDBF->pszStringField = (char *) SfRealloc(psDBF->pszStringField,psDBF->nStringFieldLen);
    }

    /* -------------------------------------------------------------------- */
    /*	Extract the requested field.					    */
    /* -------------------------------------------------------------------- */
    strncpy( psDBF->pszStringField, pabyRec+psDBF->panFieldOffset[iField], psDBF->panFieldSize[iField] );
    psDBF->pszStringField[psDBF->panFieldSize[iField]] = '\0';

    /*
    ** Trim trailing blanks (SDL Modification)
    */
    for(i=strlen(psDBF->pszStringField)-1;i>=0;i--) {
      if(psDBF->pszStringField[i] != ' ') {
	psDBF->pszStringField[i+1] = '\0';
	break;
      }
    }

    if(i == -1) psDBF->pszStringField[0] = '\0'; // whole string is blank (SDL fix)

    /*
    ** Trim/skip leading blanks (SDL/DM Modification - only on numeric types)
    */
    if( psDBF->pachFieldType[iField] == 'N' || psDBF->pachFieldType[iField] == 'F' || psDBF->pachFieldType[iField] == 'D' ) {
        for(i=0;i<(int)strlen(psDBF->pszStringField);i++) {
            if(psDBF->pszStringField[i] != ' ')
                break;
        }
        pReturnField = psDBF->pszStringField+i;
    }
    else
        pReturnField = psDBF->pszStringField;

    return( pReturnField );
}

/************************************************************************/
/*                        msDBFReadIntAttribute()                       */
/*                                                                      */
/*      Read an integer attribute.                                      */
/************************************************************************/
int	msDBFReadIntegerAttribute( DBFHandle psDBF, int iRecord, int iField )

{
  return(atoi(msDBFReadAttribute( psDBF, iRecord, iField )));
}

/************************************************************************/
/*                        msDBFReadDoubleAttribute()                    */
/*                                                                      */
/*      Read a double attribute.                                        */
/************************************************************************/
double	msDBFReadDoubleAttribute( DBFHandle psDBF, int iRecord, int iField )
{
  return(atof(msDBFReadAttribute( psDBF, iRecord, iField )));
}

/************************************************************************/
/*                        msDBFReadStringAttribute()                      */
/*                                                                      */
/*      Read a string attribute.                                        */
/************************************************************************/
const char *msDBFReadStringAttribute( DBFHandle psDBF, int iRecord, int iField )
{
  return( msDBFReadAttribute( psDBF, iRecord, iField ) );
}

/************************************************************************/
/*                          msDBFGetFieldCount()                        */
/*                                                                      */
/*      Return the number of fields in this table.                      */
/************************************************************************/
int	msDBFGetFieldCount( DBFHandle psDBF )
{
  return( psDBF->nFields );
}

/************************************************************************/
/*                         msDBFGetRecordCount()                        */
/*                                                                      */
/*      Return the number of records in this table.                     */
/************************************************************************/
int	msDBFGetRecordCount( DBFHandle psDBF )
{
  return( psDBF->nRecords );
}

/************************************************************************/
/*                          msDBFGetFieldInfo()                         */
/*                                                                      */
/*      Return any requested information about the field.               */
/************************************************************************/
DBFFieldType msDBFGetFieldInfo( DBFHandle psDBF, int iField, char * pszFieldName, int * pnWidth, int * pnDecimals )
{
  if( iField < 0 || iField >= psDBF->nFields )
    return( FTInvalid );

  if( pnWidth != NULL )
    *pnWidth = psDBF->panFieldSize[iField];

  if( pnDecimals != NULL )
    *pnDecimals = psDBF->panFieldDecimals[iField];

  if( pszFieldName != NULL )
    {
      int	i;

      strncpy( pszFieldName, (char *) psDBF->pszHeader+iField*32, 11 );
      pszFieldName[11] = '\0';
      for( i = 10; i > 0 && pszFieldName[i] == ' '; i-- )
	pszFieldName[i] = '\0';
    }

  if( psDBF->pachFieldType[iField] == 'N'
      || psDBF->pachFieldType[iField] == 'F'
      || psDBF->pachFieldType[iField] == 'D' )
    {
      if( psDBF->panFieldDecimals[iField] > 0 )
	return( FTDouble );
      else
	return( FTInteger );
    }
  else
    {
      return( FTString );
    }
}

/************************************************************************/
/*                         msDBFWriteAttribute()                        */
/*									*/
/*	Write an attribute record to the file.				*/
/************************************************************************/
static int
msDBFWriteAttribute(DBFHandle psDBF, int hEntity, int iField,
                    const void *pValue)
{
  int	       	nRecordOffset, i, j;
  uchar	*pabyRec;
  char	szSField[40], szFormat[12];

  /* -------------------------------------------------------------------- */
  /*	Is this a valid record?						  */
  /* -------------------------------------------------------------------- */
  if( hEntity < 0 || hEntity > psDBF->nRecords )
    return( MS_FALSE );

  if( psDBF->bNoHeader )
    writeHeader(psDBF);

  /* -------------------------------------------------------------------- */
  /*      Is this a brand new record?                                     */
  /* -------------------------------------------------------------------- */
  if( hEntity == psDBF->nRecords )
    {
      flushRecord( psDBF );

      psDBF->nRecords++;
      for( i = 0; i < psDBF->nRecordLength; i++ )
	psDBF->pszCurrentRecord[i] = ' ';

      psDBF->nCurrentRecord = hEntity;
    }

  /* -------------------------------------------------------------------- */
  /*      Is this an existing record, but different than the last one     */
  /*      we accessed?                                                    */
  /* -------------------------------------------------------------------- */
  if( psDBF->nCurrentRecord != hEntity )
    {
      flushRecord( psDBF );

      nRecordOffset = psDBF->nRecordLength * hEntity + psDBF->nHeaderLength;

      fseek( psDBF->fp, nRecordOffset, 0 );
      fread( psDBF->pszCurrentRecord, psDBF->nRecordLength, 1, psDBF->fp );

      psDBF->nCurrentRecord = hEntity;
    }

  pabyRec = (uchar *) psDBF->pszCurrentRecord;

  /* -------------------------------------------------------------------- */
  /*      Assign all the record fields.                                   */
  /* -------------------------------------------------------------------- */
  switch( psDBF->pachFieldType[iField] ) {
  case 'D':
  case 'N':
  case 'F':
    if( psDBF->panFieldDecimals[iField] == 0 ) {
      sprintf( szFormat, "%%%dd", psDBF->panFieldSize[iField] );
      sprintf(szSField, szFormat, (int) *((const double *) pValue) );
      if( strlen(szSField) > (unsigned)psDBF->panFieldSize[iField] )
	szSField[psDBF->panFieldSize[iField]] = '\0';
      strncpy((char *) (pabyRec+psDBF->panFieldOffset[iField]), szSField, strlen(szSField) );
    } else {
      sprintf( szFormat, "%%%d.%df", psDBF->panFieldSize[iField], psDBF->panFieldDecimals[iField] );
      sprintf(szSField, szFormat, *((const double *) pValue) );
      if( strlen(szSField) > (unsigned)psDBF->panFieldSize[iField] )
	szSField[psDBF->panFieldSize[iField]] = '\0';
      strncpy((char *) (pabyRec+psDBF->panFieldOffset[iField]),  szSField, strlen(szSField) );
    }
    break;

  default:
    if (strlen((const char *) pValue) > (unsigned)psDBF->panFieldSize[iField])
      j = psDBF->panFieldSize[iField];
    else
      j = strlen((const char *) pValue);

    strncpy((char *) (pabyRec+psDBF->panFieldOffset[iField]),
            (const char *) pValue, j);
    break;
  }

  psDBF->bCurrentRecordModified = MS_TRUE;
  psDBF->bUpdated = MS_TRUE;

  return( MS_TRUE );
}

/************************************************************************/
/*                      msDBFWriteDoubleAttribute()                     */
/*                                                                      */
/*      Write a double attribute.                                       */
/************************************************************************/
int msDBFWriteDoubleAttribute( DBFHandle psDBF, int iRecord, int iField, double dValue )
{
  return( msDBFWriteAttribute( psDBF, iRecord, iField, (void *) &dValue ) );
}

/************************************************************************/
/*                      msDBFWriteIntegerAttribute()                    */
/*                                                                      */
/*      Write a integer attribute.                                      */
/************************************************************************/

int msDBFWriteIntegerAttribute( DBFHandle psDBF, int iRecord, int iField,	int nValue )
{
  double	dValue = nValue;

  return( msDBFWriteAttribute( psDBF, iRecord, iField, (void *) &dValue ) );
}

/************************************************************************/
/*                      msDBFWriteStringAttribute()                     */
/*                                                                      */
/*      Write a string attribute.                                       */
/************************************************************************/
int msDBFWriteStringAttribute( DBFHandle psDBF, int iRecord, int iField, const char * pszValue )
{
  return msDBFWriteAttribute(psDBF, iRecord, iField, (const void *)pszValue);
}

static int
m_strcasecmp(const char *s1, const char*s2)
{
  unsigned int i;

  for (i = 0; s1[i] != 0 && s2[i] != 0; i++) {
    unsigned char x1 = tolower(s1[i]);
    unsigned char x2 = tolower(s2[i]);
    if (x1 > x2) {
      return 1;
    } else if (x1 < x2) {
      return -1;
    }
  }
  if ((0 == s1[i]) && (0 == s2[i])) {
    return 0;
  }
  if (s1[i] != 0) {
    return 1;
  }
  return -1;
}

/*
** Which column number in the .DBF file does the item correspond to
*/
int msDBFGetItemIndex(DBFHandle dbffile, char *name)
{
  int i;
  DBFFieldType dbfField;
  int fWidth,fnDecimals; /* field width and number of decimals */
  char fName[32]; /* field name */

  if(!name) {
    msSetError(MS_MISCERR, "NULL item name passed.", "msGetItemIndex()");
    return(-1);
  }

  /* does name exist as a field? */
  for(i=0;i<msDBFGetFieldCount(dbffile);i++) {
    dbfField = msDBFGetFieldInfo(dbffile,i,fName,&fWidth,&fnDecimals);
    if(m_strcasecmp(name,fName) == 0) /* found it */
      return(i);
  }

  msSetError(MS_DBFERR, "Item '%s' not found.", "msDBFGetItemIndex()",name);
  return(-1); /* item not found */
}

/*
** Load item names into a character array
*/
char **msDBFGetItems(DBFHandle dbffile)
{
  char **items;
  int i, nFields;
  char fName[32];

  if((nFields = msDBFGetFieldCount(dbffile)) == 0) {
    msSetError(MS_DBFERR, "File contains no data.", "msGetDBFItems()");
    return(NULL);
  }

  if((items = (char **)malloc(sizeof(char *)*nFields)) == NULL) {
    msSetError(MS_MEMERR, NULL, "msGetDBFItems()");
    return(NULL);
  }

  for(i=0;i<nFields;i++) {
    msDBFGetFieldInfo(dbffile, i, fName, NULL, NULL);
    items[i] = strdup(fName);
  }

  return(items);
}

/*
** Load item values into a character array
*/
char **msDBFGetValues(DBFHandle dbffile, int record)
{
  char **values;
  int i, nFields;

  if((nFields = msDBFGetFieldCount(dbffile)) == 0) {
    msSetError(MS_DBFERR, "File contains no data.", "msGetDBFValues()");
    return(NULL);
  }

  if((values = (char **)malloc(sizeof(char *)*nFields)) == NULL) {
    msSetError(MS_MEMERR, NULL, "msGetAllDBFValues()");
    return(NULL);
  }

  for(i=0;i<nFields;i++)
    values[i] = strdup(msDBFReadStringAttribute(dbffile, record, i));

  return(values);
}

int *msDBFGetItemIndexes(DBFHandle dbffile, char **items, int numitems)
{
  int *itemindexes=NULL, i;

  if(numitems == 0) return(NULL);

  itemindexes = (int *)malloc(sizeof(int)*numitems);
  if(!itemindexes) {
    msSetError(MS_MEMERR, NULL, "msGetItemIndexes()");
    return(NULL);
  }

  for(i=0;i<numitems;i++) {
    itemindexes[i] = msDBFGetItemIndex(dbffile, items[i]);
    if(itemindexes[i] == -1) {
      free(itemindexes);
      return(NULL); // item not found
    }
  }

  return(itemindexes);
}

char **msDBFGetValueList(DBFHandle dbffile, int record, int *itemindexes, int numitems)
{
  const char *value;
  char **values=NULL;
  int i;

  if(numitems == 0) return(NULL);

  if((values = (char **)malloc(sizeof(char *)*numitems)) == NULL) {
    msSetError(MS_MEMERR, NULL, "msGetSomeDBFValues()");
    return(NULL);
  }

  for(i=0;i<numitems;i++) {
    value = msDBFReadStringAttribute(dbffile, record, itemindexes[i]);
    if (value == NULL)
      return NULL; /* Error already reported by msDBFReadStringAttribute() */
    values[i] = strdup(value);
  }

  return(values);
}