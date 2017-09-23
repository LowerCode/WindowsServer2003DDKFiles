/*++

Copyright (c) 1990-2003  Microsoft Corporation


Module Name:

    docprop.c


Abstract:

    This module contains functions for DrvDocumentPropertySheets


Author:

    07-Dec-1993 Tue 12:15:40 created  


[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/

#include "precomp.h"
#pragma hdrstop


#define DBG_PLOTFILENAME    DbgDocProp


extern HMODULE  hPlotUIModule;


#define DBG_DP_SETUP        0x00000001
#define DBG_DP_FORM         0x00000002
#define DBG_HELP            0x00000004

DEFINE_DBGVAR(0);



OPDATA  OPOrientation[] = {

            { 0, IDS_CPSUI_PORTRAIT,  IDI_CPSUI_PORTRAIT,  0, 0, 0   },
            { 0, IDS_CPSUI_LANDSCAPE, IDI_CPSUI_LANDSCAPE, 0, 0, 0   }
        };

OPDATA  OPColor[] = {

            { 0, IDS_CPSUI_MONOCHROME, IDI_CPSUI_MONO,  0, 0, 0   },
            { 0, IDS_CPSUI_COLOR,      IDI_CPSUI_COLOR, 0, 0, 0   }
        };

OPDATA  OPCopyCollate[] = {

            { 0, IDS_CPSUI_COPIES, IDI_CPSUI_COPY, 0, 0,   0 },
            { 0, 0,                1,              0, 0, 100 }
        };

OPDATA  OPScaling[] = {

            { 0, IDS_CPSUI_PERCENT, IDI_CPSUI_SCALING, 0, 0,   0 },
            { 0, 0,                 1,                 0, 0, 100 }
        };

OPDATA  OPPrintQuality[] = {

            { 0, IDS_QUALITY_DRAFT,  IDI_CPSUI_RES_DRAFT,        0, 0, -1 },
            { 0, IDS_QUALITY_LOW,    IDI_CPSUI_RES_LOW,          0, 0, -2 },
            { 0, IDS_QUALITY_MEDIUM, IDI_CPSUI_RES_MEDIUM,       0, 0, -3 },
            { 0, IDS_QUALITY_HIGH,   IDI_CPSUI_RES_PRESENTATION, 0, 0, -4 }
        };

OPDATA  OPHTClrAdj = {

            0,
            PI_OFF(PlotDM) + PLOTDM_OFF(ca),
            IDI_CPSUI_HTCLRADJ,
            PUSHBUTTON_TYPE_HTCLRADJ,
            0,
            0
        };


extern OPDATA  OPNoYes[];

OPDATA  OPFillTrueType[] = {

            { 0, IDS_CPSUI_NO,  IDI_FILL_TRUETYPE_NO,  0,  0, 0  },
            { 0, IDS_CPSUI_YES, IDI_FILL_TRUETYPE_YES, 0,  0, 0  }
        };


OIDATA  DPOIData[] = {

    {
        ODF_PEN_RASTER | ODF_CALLCREATEOI,
        0,
        OI_LEVEL_1,
        DMPUB_FORMNAME,
        TVOT_LISTBOX,
        IDS_CPSUI_FORMNAME,
        0,
        IDH_FORMNAME,
        0,
        (POPDATA)CreateFormOI
    },

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_1,
        DMPUB_ORIENTATION,
        TVOT_2STATES,
        IDS_CPSUI_ORIENTATION,
        0,
        IDH_ORIENTATION,
        COUNT_ARRAY(OPOrientation),
        OPOrientation
    },

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_1,
        DMPUB_COPIES_COLLATE,
        TVOT_UDARROW,
        IDS_CPSUI_NUM_OF_COPIES,
        0,
        IDH_COPIES_COLLATE,
        COUNT_ARRAY(OPCopyCollate),
        OPCopyCollate
    },

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_1,
        DMPUB_PRINTQUALITY,
        TVOT_LISTBOX,
        IDS_CPSUI_PRINTQUALITY,
        0,
        IDH_PRINTQUALITY,
        COUNT_ARRAY(OPPrintQuality),
        OPPrintQuality
    },

    {
        ODF_RASTER | ODF_COLOR,
        0,
        OI_LEVEL_1,
        DMPUB_COLOR,
        TVOT_2STATES,
        IDS_CPSUI_COLOR_APPERANCE,
        0,
        IDH_COLOR,
        COUNT_ARRAY(OPColor),
        OPColor
    },

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_1,
        DMPUB_SCALE,
        TVOT_UDARROW,
        IDS_CPSUI_SCALING,
        0,
        IDH_SCALE,
        COUNT_ARRAY(OPScaling),
        OPScaling
    },

    {
        ODF_RASTER,
        0,
        OI_LEVEL_1,
        DP_HTCLRADJ,
        TVOT_PUSHBUTTON,
        IDS_CPSUI_HTCLRADJ,
        OTS_PUSH_ENABLE_ALWAYS,
        IDH_HTCLRADJ,
        1,
        &OPHTClrAdj
    },

    {
        ODF_PEN,
        0,
        OI_LEVEL_1,
        DP_FILL_TRUETYPE,
        TVOT_2STATES,
        IDS_FILL_TRUETYPE,
        0,
        IDH_FILL_TRUETYPE,
        2,
        OPFillTrueType
    },

    {
        ODF_RASTER,
        0,
        OI_LEVEL_1,
        DP_QUICK_POSTER_MODE,
        TVOT_2STATES,
        IDS_POSTER_MODE,
        0,
        IDH_POSTER_MODE,
        2,
        OPNoYes
    }
};



UINT
SetupDPOptItems(
    PPRINTERINFO    pPI
    )

/*++

Routine Description:




Arguments:




Return Value:




Author:

    16-Nov-1995 Thu 14:15:25 created  


Revision History:


--*/

{
    PPLOTDEVMODE    pPlotDM;
    PPLOTGPC        pPlotGPC;
    POPTITEM        pOptItem;
    POPTITEM        pOI;
    POIDATA         pOIData;
    DWORD           Flags;
    DWORD           ODFlags;
    UINT            i;



    pOI      =
    pOptItem = pPI->pOptItem;
    pOIData  = DPOIData;
    i        = (UINT)COUNT_ARRAY(DPOIData);
    pPlotGPC = pPI->pPlotGPC;
    pPlotDM  = &(pPI->PlotDM);
    Flags    = pPlotGPC->Flags;
    ODFlags  = (Flags & PLOTF_RASTER) ? ODF_RASTER : ODF_PEN;

    while (i--) {

        DWORD   OIFlags = pOIData->Flags;


        switch (pOIData->DMPubID) {

        case DMPUB_COPIES_COLLATE:

            if (pPlotGPC->MaxCopies <= 1) {

                OIFlags = 0;
            }

            break;

        case DMPUB_SCALE:

            if (!pPlotGPC->MaxScale) {

                OIFlags = 0;
            }

            break;
        }

        if ((!(OIFlags & ODFlags))                                      ||
            ((OIFlags & ODF_ROLLFEED) && (!(Flags & PLOTF_ROLLFEED)))   ||
            ((OIFlags & ODF_NO_PAPERTRAY) && (Flags & PLOTF_PAPERTRAY)) ||
            ((OIFlags & ODF_COLOR) && (!(Flags & PLOTF_COLOR)))) {

            //
            // Nothing to do here
            //

            NULL;

        } else if (OIFlags & ODF_CALLCREATEOI) {

            pOI += pOIData->pfnCreateOI(pPI,
                                        (LPVOID)((pOptItem) ? pOI : NULL),
                                        pOIData);

        } else if (pOptItem) {

            if (CreateOPTTYPE(pPI, pOI, pOIData, pOIData->cOPData, NULL)) {

                POPTPARAM   pOP = pOI->pOptType->pOptParam;

                switch (pOI->DMPubID) {

                case DMPUB_ORIENTATION:

                    pOI->Sel = (LONG)((pPlotDM->dm.dmOrientation ==
                                                DMORIENT_PORTRAIT) ? 0 : 1);
                    break;

                case DMPUB_COPIES_COLLATE:

                    pOP[1].lParam = (LONG)pPlotGPC->MaxCopies;
                    pOI->Sel      = (LONG)pPlotDM->dm.dmCopies;
                    break;

                case DMPUB_PRINTQUALITY:

                    switch (pPlotGPC->MaxQuality) {

                    case 0:
                    case 1:

                        pPlotDM->dm.dmPrintQuality = DMRES_HIGH;
                        pOP[0].Flags |= OPTPF_HIDE;

                    case 2:

                        pOP[2].Flags |= OPTPF_HIDE;

                    case 3:

                        pOP[1].Flags |= OPTPF_HIDE;
                        break;

                    default:

                        break;
                    }

                    pOI->Sel = (LONG)-(pPlotDM->dm.dmPrintQuality -
                                                                DMRES_DRAFT);
                    break;

                case DMPUB_COLOR:

                    pOI->Sel = (LONG)((pPlotDM->dm.dmColor == DMCOLOR_COLOR) ?
                                                1 : 0);
                    break;

                case DMPUB_SCALE:

                    pOP[1].lParam = (LONG)pPlotGPC->MaxScale;
                    pOI->Sel      = (LONG)pPlotDM->dm.dmScale;
                    break;

                case DP_FILL_TRUETYPE:

                    pOI->Sel = (LONG)((pPlotDM->Flags & PDMF_FILL_TRUETYPE) ?
                                                                        1 : 0);
                    break;


                case DP_QUICK_POSTER_MODE:

                    pOI->Sel = (LONG)((pPlotDM->Flags & PDMF_PLOT_ON_THE_FLY) ?
                                                                        1 : 0);
                    break;
                }

                pOI++;
            }

        } else {

            pOI++;
        }

        pOIData++;
    }

    if ((i = (UINT)(pOI - pOptItem)) && (!pOptItem)) {

        if (pPI->pOptItem = (POPTITEM)LocalAlloc(LPTR, sizeof(OPTITEM) * i)) {

            pPI->cOptItem = (WORD)i;

            //
            // Call myself second time to really create it
            //

            SetupDPOptItems(pPI);

        } else {

            i = 0;

            PLOTERR(("GetPPpOptItem(): LocalAlloc(%ld) failed",
                                            sizeof(OPTITEM) * i));
        }
    }

    return(i);
}



VOID
SaveDPOptItems(
    PPRINTERINFO    pPI
    )

/*++

Routine Description:

    This function save all the device options back to registry if one changed
    and has a update permission


Arguments:

    pPI     - Pointer to the PRINTERINFO


Return Value:

    VOID


Author:

    06-Nov-1995 Mon 18:05:16 created  


Revision History:


--*/

{
    POPTITEM        pOI;
    POPTITEM        pLastItem;
    PPLOTDEVMODE    pPlotDM;
    BYTE            DMPubID;


    pOI       = pPI->pOptItem;
    pLastItem = pOI + pPI->cOptItem - 1;
    pPlotDM   = &(pPI->PlotDM);

    while (pOI <= pLastItem) {

        if (pOI->Flags & OPTIF_CHANGEONCE) {

            switch (pOI->DMPubID) {

            case DMPUB_FORMNAME:

                GetFormSelect(pPI, pOI);
                break;

            case DMPUB_ORIENTATION:

                pPlotDM->dm.dmOrientation = (SHORT)((pOI->Sel) ?
                                                        DMORIENT_LANDSCAPE :
                                                        DMORIENT_PORTRAIT);
                break;

            case DMPUB_COPIES_COLLATE:

                pPlotDM->dm.dmCopies = (SHORT)pOI->Sel;
                break;

            case DMPUB_PRINTQUALITY:

                pPlotDM->dm.dmPrintQuality = (SHORT)(-(pOI->Sel) + DMRES_DRAFT);
                break;

            case DMPUB_COLOR:

                pPlotDM->dm.dmColor = (SHORT)((pOI->Sel) ? DMCOLOR_COLOR :
                                                           DMCOLOR_MONOCHROME);
                break;

            case DMPUB_SCALE:

                pPlotDM->dm.dmScale = (SHORT)pOI->Sel;
                break;

            case DP_FILL_TRUETYPE:

                if (pOI->Sel) {

                    pPlotDM->Flags |= PDMF_FILL_TRUETYPE;

                } else {

                    pPlotDM->Flags &= ~PDMF_FILL_TRUETYPE;
                }

                break;

            case DP_QUICK_POSTER_MODE:

                if (pOI->Sel) {

                    pPlotDM->Flags |= PDMF_PLOT_ON_THE_FLY;

                } else {

                    pPlotDM->Flags &= ~PDMF_PLOT_ON_THE_FLY;
                }

                break;
            }
        }

        pOI++;
    }
}




CPSUICALLBACK
DPCallBack(
    PCPSUICBPARAM   pCPSUICBParam
    )

/*++

Routine Description:

    This is the callback function from the common property sheet UI


Arguments:

    pCPSUICBParam   - Pointer to the CPSUICBPARAM data structure to describe
                      the nature of the callback


Return Value:

    LONG


Author:

    07-Nov-1995 Tue 15:15:02 created  


Revision History:


--*/

{
    POPTITEM    pCurItem = pCPSUICBParam->pCurItem;
    LONG        Action = CPSUICB_ACTION_NONE;

    if (pCPSUICBParam->Reason == CPSUICB_REASON_APPLYNOW) {

        PPRINTERINFO    pPI = (PPRINTERINFO)pCPSUICBParam->UserData;

        if ((pPI->Flags & PIF_UPDATE_PERMISSION) &&
            (pPI->pPlotDMOut)) {

            SaveDPOptItems(pPI);

            PLOTDBG(DBG_DP_SETUP, ("APPLYNOW: ConvertDevmodeOut"));

            ConvertDevmodeOut((PDEVMODE)&(pPI->PlotDM),
                              (PDEVMODE)pPI->pPlotDMIn,
                              (PDEVMODE)pPI->pPlotDMOut);

            pCPSUICBParam->Result = CPSUI_OK;
            Action                = CPSUICB_ACTION_ITEMS_APPLIED;
        }
    }

    return(Action);
}




BOOL
DrvConvertDevMode(
    LPTSTR      pPrinterName,
    PDEVMODE    pDMIn,
    PDEVMODE    pDMOut,
    PLONG       pcbNeeded,
    DWORD       fMode
    )

/*++

Routine Description:

    This function is used by the SetPrinter() and GetPrinter() spooler calls.

Arguments:

    pPrinterName    - Points to printer name string

    pDMIn           - Points to the input devmode

    pDMOut          - Points to the output devmode buffer

    pcbNeeded       - Specifies the size of output buffer on input On output,
                      this is the size of output devmode

    fMode           - Specifies what function to perform


Return Value:

    TRUE if successful
    FALSE otherwise and an error code is logged

Author:

    08-Jan-1996 Mon 12:40:22 created  


Revision History:


--*/

{
    DWORD                       cb;
    INT                         Result;
    static DRIVER_VERSION_INFO  PlotDMVersions = {

        DRIVER_VERSION, PLOTDM_PRIV_SIZE,   // current version/size
        0x0350,         PLOTDM_PRIV_SIZE,   // NT3.51 version/size
    };

    //
    // Call a library routine to handle the common cases
    //

    Result = CommonDrvConvertDevmode(pPrinterName,
                                     pDMIn,
                                     pDMOut,
                                     pcbNeeded,
                                     fMode,
                                     &PlotDMVersions);

    //
    // If not handled by the library routine, we only need to worry
    // about the case when fMode is CDM_DRIVER_DEFAULT
    //

    if ((Result == CDM_RESULT_NOT_HANDLED)  &&
        (fMode == CDM_DRIVER_DEFAULT)) {

        HANDLE  hPrinter;

        if (OpenPrinter(pPrinterName, &hPrinter, NULL)) {

            PPLOTGPC    pPlotGPC;

            if (pPlotGPC = hPrinterToPlotGPC(hPrinter, pDMOut->dmDeviceName, CCHOF(pDMOut->dmDeviceName))) {

                SetDefaultPLOTDM(hPrinter,
                                 pPlotGPC,
                                 pDMOut->dmDeviceName,
                                 (PPLOTDEVMODE)pDMOut,
                                 NULL);

                UnGetCachedPlotGPC(pPlotGPC);
                Result = CDM_RESULT_TRUE;

            } else {

                PLOTERR(("DrvConvertDevMode: hPrinterToPlotGPC(%ws) failed.",
                                                                pPrinterName));

                SetLastError(ERROR_INVALID_DATA);
            }

            ClosePrinter(hPrinter);

        } else {

            PLOTERR(("DrvConvertDevMode: OpenPrinter(%ws) failed.",
                                                            pPrinterName));
            SetLastError(ERROR_INVALID_DATA);
        }
    }

    return(Result == CDM_RESULT_TRUE);
}



LONG
DrvDocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    )

/*++

Routine Description:

    Show document property dialog box and update the output DEVMODE


Arguments:

    pPSUIInfo   - Pointer to the PROPSHEETUI_INFO data structure

    lParam      - LPARAM for this call, it is a pointer to the
                  DOCUMENTPROPERTYHEADER

Return Value:

    LONG, 1=successful, 0=failed.


Author:

    02-Feb-1996 Fri 10:47:42 created  


Revision History:


--*/

{
    PDOCUMENTPROPERTYHEADER     pDPHdr;
    PPRINTERINFO                pPI;
    LONG_PTR                    Result;


    //
    // Assume faild first
    //

    Result = -1;

    if (pPSUIInfo) {

        if (!(pDPHdr = (PDOCUMENTPROPERTYHEADER)pPSUIInfo->lParamInit)) {

            PLOTERR(("DrvDocumentPropertySheets: Pass a NULL lParamInit"));
            return(-1);
        }

    } else {

        if (pDPHdr = (PDOCUMENTPROPERTYHEADER)lParam) {

            //
            // We do not have pPSUIInfo, so that we assume this is call
            // directly from the spooler and lParam is the pDPHdr
            //

            if ((pDPHdr->fMode == 0) || (pDPHdr->pdmOut == NULL)) {

                Result = (pDPHdr->cbOut = sizeof(PLOTDEVMODE));

            } else if ((pDPHdr->fMode & (DM_COPY | DM_UPDATE))  &&
                       (!(pDPHdr->fMode & DM_NOPERMISSION))     &&
                       (pDPHdr->pdmOut)) {
                //
                // The MapPrinter will allocate memory, set default devmode,
                // reading and validating the GPC then update from current pritner
                // registry, it also will cached the pPI.
                //

                if (pPI = MapPrinter(pDPHdr->hPrinter,
                                     (PPLOTDEVMODE)pDPHdr->pdmIn,
                                     NULL,
                                     0)) {

                    ConvertDevmodeOut((PDEVMODE)&(pPI->PlotDM),
                                      (PDEVMODE)pDPHdr->pdmIn,
                                      (PDEVMODE)pDPHdr->pdmOut);

                    Result = 1;
                    UnMapPrinter(pPI);

                } else {

                    PLOTRIP(("DrvDocumentPropertySheets: MapPrinter() failed"));
                }

            } else {

                Result = 1;
            }

        } else {

            PLOTRIP(("DrvDocumentPropertySheets: ??? pDPHdr (lParam) = NULL"));
        }

        return((LONG)Result);
    }

    //
    // Now, this is the call from common UI, assume error to start with
    //

    if (pPSUIInfo->Reason == PROPSHEETUI_REASON_INIT) {

        if (!(pPI = MapPrinter(pDPHdr->hPrinter,
                               (PPLOTDEVMODE)pDPHdr->pdmIn,
                               NULL,
                               MPF_HELPFILE | MPF_PCPSUI))) {

            PLOTRIP(("DrvDocumentPropertySheets: MapPrinter() failed"));

            SetLastError(ERROR_INVALID_DATA);
            return(ERR_CPSUI_GETLASTERROR);
        }

        if ((pDPHdr->fMode & (DM_COPY | DM_UPDATE))  &&
            (!(pDPHdr->fMode & DM_NOPERMISSION))     &&
            (pDPHdr->pdmOut)) {

            pPI->Flags      = (PIF_DOCPROP | PIF_UPDATE_PERMISSION);
            pPI->pPlotDMOut = (PPLOTDEVMODE)pDPHdr->pdmOut;

        } else {

            pPI->Flags      = PIF_DOCPROP;
            pPI->pPlotDMOut = NULL;
        }

        //
        // We need to display something to let user modify/update, we wll check
        // which document properties dialog box to be used
        //
        // The return value either IDOK or IDCANCEL
        //

        pPI->pCPSUI->Flags       = 0;
        pPI->pCPSUI->pfnCallBack = DPCallBack;
        pPI->pCPSUI->pDlgPage    = (pDPHdr->fMode & DM_ADVANCED) ?
                                                 CPSUI_PDLGPAGE_ADVDOCPROP :
                                                 CPSUI_PDLGPAGE_DOCPROP;

        Result = (LONG_PTR)SetupDPOptItems(pPI);

    } else {

        pPI    = (PPRINTERINFO)pPSUIInfo->UserData;
        Result = (LONG_PTR)pDPHdr->pszPrinterName;
    }

    return(DefCommonUIFunc(pPSUIInfo, lParam, pPI, Result));
}


