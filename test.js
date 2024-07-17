function myTB251100_toss(nodeId, type, enterPath) {
    var pathMaster = '/forms9/7';
    if (enterPath)
        pathMaster = enterPath;
    var query = db.getAPIQuery(pathMaster);
    var entry;
    var strOut = "";
    var fromNumberMaster = 0;
    var resultsMaste = [1, 2, 3];
    function queryTOresults() {
        /*
        if (type == 9) {
            query = db.getAPIQuery(pathMaster);
            query.addFilter(1005701, 'regex', regexStr);//是否未過期
            query.setLimitFrom(fromNumberMaster);

            resultsMaste = query.getAPIResultList();
            fromNumberMaster += resultsMaste.length;
        }*/
    }
    queryTOresults();
    var breakSwitch = false;
    while (resultsMaste.length > 0 && !breakSwitch) {
        //默認讀取全部資料除非breakSwitch=1
        for (var iResults = 0; iResults < resultsMaste.length && !breakSwitch; iResults++) {
            strOut = "";
            switch (type) {
                //全部刷新
                case 9:
                    entry = resultsMaste[iResults];
                    break;
                //單筆刷新
                default:
                    query = db.getAPIQuery(pathMaster);
                    entry = query.getAPIEntry(nodeId);
                    breakSwitch = true;
                    break;
            }
            switch (type) {
                case 1:
                case 2:
                case 3:
                    var Field_Link = 0, Field_Number = 0, Field_Value = 0, Field_OPR = 0,
                        Field_Supplier = 0, Field_Date = 0, Field_NO = 0, Field_OutDate = 0,
                        Field_PO = 0, Field_MO = 0;
                    var Field_Path = "";
                    //依據type帶入各表單FieldID
                    switch (type) {
                        case 1:
                            if (entry.getFieldValue(1010964) != "") {
                                response.setMessage("已拋轉過!");
                                return;
                            }
                            Field_Link = 1011844;
                            Field_Number = 1003991;
                            Field_Value = 1003994;
                            Field_OPR = 1002359;
                            Field_Supplier = 1002366;
                            Field_Date = 1004017;
                            Field_NO = 1002355;
                            Field_Path = '/forms9/3';//採購單
                            break;
                        case 2:
                            if (entry.getFieldValue(1010965) != "") {
                                response.setMessage("已拋轉過!");
                                return;
                            }
                            Field_Link = 1011843;
                            Field_Number = 1004179;
                            Field_Value = 1004181;
                            Field_OPR = 1007549;
                            Field_Supplier = 1004165;
                            Field_Date = 1004184;
                            Field_NO = 1004156;
                            Field_OutDate = 1012466;
                            Field_PO = 1012468;
                            Field_MO = 1012497;
                            Field_Path = '/forms9/22';    //委外加工單                                
                            break;
                        case 3:
                            if (entry.getFieldValue(1012092) != "") {
                                response.setMessage("已拋轉過!");
                                return;
                            }
                            Field_Link = 1011842;
                            Field_Number = 1007612;
                            Field_Value = 1007620;
                            Field_OPR = 1007606;
                            Field_Supplier = 1007576;
                            Field_Date = 1007616;
                            Field_NO = 1007569;
                            Field_Path = '/forms9/35';//通知廠商出貨單     
                            break;

                    }
                    var arrSubtable = [];
                    for (var i = 0, numLine = -100, SubtableID = 1003130; i < entry.getSubtableSize(SubtableID); i++, numLine--) {
                        //將編號存入陣列        
                        arrSubtable[i] = {
                            'OPR': entry.getFieldValue(1003115),//請購單號
                            'Number': entry.getSubtableFieldValue(SubtableID, i, 1003124),//商品編號
                            'Value': entry.getSubtableFieldValue(SubtableID, i, 1003127),//需求數量
                            'Use': entry.getSubtableFieldValue(SubtableID, i, 1003128),//用途
                            'Date': entry.getSubtableFieldValue(SubtableID, i, 1003133),//期望到貨日
                            'Level': entry.getSubtableFieldValue(SubtableID, i, 1003134),//緊急程度
                            'OutDate': entry.getSubtableFieldValue(SubtableID, i, 1012486),//發料日期
                            'Supplier': entry.getSubtableFieldValue(SubtableID, i, 1011841),//供應商編號    
                            'PO': entry.getSubtableFieldValue(SubtableID, i, 1012487),//訂單號碼   
                            'MO': entry.getFieldValue(1012494),//生產單號
                        };
                        //如果之前沒拋過則新增資料
                        if (entry.getSubtableFieldValue(SubtableID, i, Field_Link) == "") {
                            var query = db.getAPIQuery(Field_Path);
                            var tossentry = query.insertAPIEntry();
                            tossentry.setSubtableFieldValue(Field_Number, numLine, arrSubtable[i]['Number']);
                            tossentry.setSubtableFieldValue(Field_Value, numLine, arrSubtable[i]['Value']);
                            tossentry.setSubtableFieldValue(Field_Date, numLine, arrSubtable[i]['Date']);
                            if (type == 2) {
                                if (arrSubtable[i]['OutDate'] == "") {
                                    response.setStatus('WARN');
                                    response.setMessage('請輸入發料日期');
                                    continue;
                                }
                                if (arrSubtable[i]['OutDate'] == "") {
                                    response.setStatus('WARN');
                                    response.setMessage('請輸入訂單編號');
                                    continue;
                                }
                                tossentry.setSubtableFieldValue(Field_OutDate, numLine, arrSubtable[i]['OutDate']);
                                tossentry.setSubtableFieldValue(Field_PO, numLine, arrSubtable[i]['PO']);
                                tossentry.setSubtableFieldValue(Field_MO, numLine, arrSubtable[i]['MO']);

                            }
                            tossentry.setSubtableFieldValue(Field_OutDate, numLine, arrSubtable[i]['OutDate']);
                            tossentry.setSubtableFieldValue(Field_PO, numLine, arrSubtable[i]['PO']);
                            tossentry.setFieldValue(Field_OPR, arrSubtable[i]['OPR']);
                            tossentry.setFieldValue(Field_Supplier, arrSubtable[i]['Supplier']);
                            tossentry.loadAllLinkAndLoad();
                            tossentry.loadAllDefaultValues(user);
                            tossentry.recalculateAllFormulas();
                            tossentry.loadAllLinkAndLoad();
                            tossentry.setIfExecuteWorkflow(true);
                            tossentry.save();

                            //tossentry = query.getAPIEntry(tossentry.getRootNodeId());
                        }
                    }
                    entry.recalculateAllFormulas();
                    entry.setIfExecuteWorkflow(true);
                    entry.save();

                    query = db.getAPIQuery(pathMaster);
                    entry = query.getAPIEntry(nodeId);
                    entry.recalculateAllFormulas();
                    entry.setIfExecuteWorkflow(true);
                    entry.save();
                    break;
                //[ ]刷新進貨資訊
                default:
                    /*
                    entry.setFieldValue(1013265,
                        "  [color=Red]" + entry.getFieldValue(1013215) +
                        IF(AND(entry.getFieldValue(1013215) != "", OR(entry.getFieldValue(1013216) != "", entry.getFieldValue(1013217) != "")), "、", "") +
                        entry.getFieldValue(1013216) + IF(AND(entry.getFieldValue(1013217) != "", entry.getFieldValue(1013216) != ""), "、", "") +
                        entry.getFieldValue(1013217) + "[/color]");
*/
                    //取最新日期
                    entry.recalculateFormula(1011004);
                    var dateNow = entry.getFieldValue(1011004);
                    var strFieldValueAll = "", strDateAll = "";
                    var arrDateAll = [];
                    //分別對3張單計算
                    for (var j = 0; j < 3; j++) {
                        var jsDate = {};
                        var strFieldValue = "";
                        switch (j) {
                            case 0:
                                jsDate["ID"] = 1013215;
                                jsDate["標題"] = "採購單:";
                                break;
                            case 1:
                                jsDate["ID"] = 1013216;
                                jsDate["標題"] = "委外加工單:";
                                break;
                            case 2:
                                jsDate["ID"] = 1013217;
                                jsDate["標題"] = "通知出貨單:";

                                break;
                        }
                        //取每項文字
                        var arrDate = String(entry.getFieldValue(jsDate["ID"]) + '、').match(/(\d+\/\d+\/\d+.*?)(?:[、|\[])/g);
                        if (arrDate) {
                            for (var i = 0; i < arrDate.length; i++) {
                                //取日期判斷並合成文字
                                arrDate[i] = arrDate[i].slice(0, -1);
                                var dataMatch = arrDate[i].match(/\d+\/\d+\/\d+/);
                                if (new Date(dataMatch).getTime() >= new Date(dateNow).getTime()) {
                                    arrDateAll.push(String(dataMatch));
                                    if (strFieldValue == "") {
                                        strFieldValue += jsDate["標題"];
                                        strFieldValue += arrDate[i];
                                    }
                                    else {
                                        strFieldValue += '、';
                                        strFieldValue += arrDate[i];
                                    }
                                }
                            }
                            if (strFieldValueAll != "")
                                strFieldValueAll += '、';
                            strFieldValueAll += strFieldValue;
                        }
                    }
                    entry.setFieldValue(1013265, "  [color=Red]" + strFieldValueAll + "[/color]");
                    if (arrDateAll.length > 0) {
                        arrDateAll = arrDateAll.filter(function (item, index, array) {
                            return array.indexOf(item) === index;
                        });
                        for (var i = 0; i < arrDateAll.length; i++) {
                            if (strDateAll != "")
                                strDateAll += '、';
                            strDateAll += arrDateAll[i];
                        }
                        // response.setMessage(strDateAll);
                        entry.setFieldValue(1013266, strDateAll);
                    }
                    entry.recalculateFormula(1008472);
                    
                     // entry.setFieldValue(1013215,"");
                //entry.recalculateFormula(1013215);
                    entry.save();
            }

        }
        queryTOresults();
    }
    response.setStatus('SUCCESS');
    // response.setMessage('執行完成');
}
function _RAGIC_AUTOGEN_FIELD_UPDATER_49009(nodeId) {
    // WARNING: Please DO NOT edit auto-generated script.
    var result = db.fieldUpdater(JSON.stringify(
{ "THIS_PATH": "/forms9/3", "THIS_NODE_ID": nodeId, "RECALCULATE": false, "CHECK_VA_UPDATED_FIELDS": false, "CHECK_VA_ALL_FIELDS": false, "SUBTABLE_UPDATE_POLICY": "FORCE_CREATE", "UPD_FIELDS": { "1007788": "Yes" } }
    ), response);
}//END_RAGIC_AUTOGEN_FIELD_UPDATER_49009

function _RAGIC_AUTOGEN_FIELD_UPDATER_40984(nodeId) {
    // WARNING: Please DO NOT edit auto-generated script.
    var result = db.fieldUpdater(JSON.stringify(
{ "THIS_PATH": "/forms9/3", "THIS_NODE_ID": nodeId, "RECALCULATE": false, "CHECK_VA_UPDATED_FIELDS": false, "CHECK_VA_ALL_FIELDS": false, "SUBTABLE_UPDATE_POLICY": "FORCE_CREATE", "UPD_FIELDS": { "1007788": "No" } }
    ), response);
}//END_RAGIC_AUTOGEN_FIELD_UPDATER_40984

function _RAGIC_AUTOGEN_FIELD_UPDATER_60095(nodeId) {
    // WARNING: Please DO NOT edit auto-generated script.
    var result = db.fieldUpdater(JSON.stringify(
{ "THIS_PATH": "/forms9/3", "THIS_NODE_ID": nodeId, "RECALCULATE": false, "CHECK_VA_UPDATED_FIELDS": false, "CHECK_VA_ALL_FIELDS": false, "SUBTABLE_UPDATE_POLICY": "FORCE_CREATE", "APPLY_SUBTABLE_FILTERS": false, "UPD_FIELDS": { "1008483": "No" } }
    ), response);
}//END_RAGIC_AUTOGEN_FIELD_UPDATER_60095

function _RAGIC_AUTOGEN_FIELD_UPDATER_59518(nodeId) {
    // WARNING: Please DO NOT edit auto-generated script.
    var result = db.fieldUpdater(JSON.stringify(
{ "THIS_PATH": "/forms9/3", "THIS_NODE_ID": nodeId, "RECALCULATE": false, "CHECK_VA_UPDATED_FIELDS": false, "CHECK_VA_ALL_FIELDS": false, "SUBTABLE_UPDATE_POLICY": "FORCE_CREATE", "APPLY_SUBTABLE_FILTERS": false, "UPD_FIELDS": { "1008483": "Yes" } }
    ), response);
}//END_RAGIC_AUTOGEN_FIELD_UPDATER_59518

function _RAGIC_AUTOGEN_FIELD_UPDATER_92538(nodeId) {
    // WARNING: Please DO NOT edit auto-generated script.
    var result = db.fieldUpdater(JSON.stringify(
{ "THIS_PATH": "/forms9/3", "THIS_NODE_ID": nodeId, "RECALCULATE": true, "CHECK_VA_UPDATED_FIELDS": false, "CHECK_VA_ALL_FIELDS": false, "SUBTABLE_UPDATE_POLICY": "FORCE_CREATE", "APPLY_SUBTABLE_FILTERS": false, "UPD_FIELDS": { "1011053": "Yes" } }
    ), response);
}//END_RAGIC_AUTOGEN_FIELD_UPDATER_92538


function _RAGIC_AUTOGEN_CR_25685(nodeId) {
    // WARNING: Please DO NOT edit auto-generated script.
    var result = db.entryCopier(JSON.stringify(
{ "THIS_PATH": "/forms9/3", "THIS_NODEID": nodeId, "NEW_PATH": "/forms9/22", "CHECK_MUST_FIELDS": false, "CHECK_VALIDATION_FIELDS": false, "CHECK_ALL_MUST_FIELDS_MAPPED": false, "SILENT": false, "RECAL_ALL_FORMULAS": true, "EXEC_WORKFLOW": true, "LOCK_SOURCE": false, "COPY": { "320": "320", "1004157": "1002356", "1004158": "1002971", "1004159": "1002357", "1004160": "1002972", "1004161": "1002362", "1004178": "1003990", "1004179": "1003991", "1004180": "1003992", "1004181": "1003994", "1004182": "1003995", "1004183": "1003996", "1004184": "1004017", "1004185": "1003997", "1004186": "1003998", "1004187": "1003999", "1004189": "1004001", "1004190": "1002390", "1004191": "1002391", "1004192": "1004001", "1004193": "1002395", "1004194": "1002396", "1004195": "1002397", "1004204": "1002836", "1004205": "1002837", "1004206": "1002838", "1004207": "1003327", "1004208": "1003328", "1004209": "1003329", "1004210": "1003330", "1006778": "1002355", "1011543": "1011542" }, "APPLY_SUBTABLE_FILTERS": false, "APPLY_SUBTABLE_GROUPING": false }
    ), response);
    if (result) return result;
}//END_RAGIC_AUTOGEN_CR_25685

function _RAGIC_AUTOGEN_CR_95121(nodeId) {
    // WARNING: Please DO NOT edit auto-generated script.
    var result = db.entryCopier(JSON.stringify(
{ "THIS_PATH": "/forms9/3", "THIS_NODEID": nodeId, "NEW_PATH": "/forms9/35", "CHECK_MUST_FIELDS": false, "CHECK_VALIDATION_FIELDS": false, "CHECK_ALL_MUST_FIELDS_MAPPED": false, "SILENT": false, "RECAL_ALL_FORMULAS": true, "EXEC_WORKFLOW": true, "LOCK_SOURCE": false, "COPY": { "1007571": "1002355", "1007574": "1002356", "1007576": "1002366", "1007577": "1002368", "1007578": "1003331", "1007592": "1003997", "1007597": "1002836", "1007598": "1002837", "1007599": "1002838", "1007606": "1002359", "1007609": "1003990", "1007611": "1003995", "1007612": "1003991", "1007613": "1003997", "1007614": "1003992", "1007616": "1004017", "1007617": "1003998", "1007619": "1003999", "1007620": "1003994", "1011544": "1011542" }, "APPLY_SUBTABLE_FILTERS": false, "APPLY_SUBTABLE_GROUPING": false }
    ), response);
    if (result) return result;
}//END_RAGIC_AUTOGEN_CR_95121


function _RAGIC_AUTOGEN_CR_72440(nodeId) {
    // WARNING: Please DO NOT edit auto-generated script.
    var result = db.entryCopier(JSON.stringify(
{ "THIS_PATH": "/forms9/3", "THIS_NODEID": nodeId, "NEW_PATH": "/-/18", "CHECK_MUST_FIELDS": false, "CHECK_VALIDATION_FIELDS": false, "CHECK_ALL_MUST_FIELDS_MAPPED": false, "SILENT": false, "RECAL_ALL_FORMULAS": true, "EXEC_WORKFLOW": true, "LOCK_SOURCE": false, "COPY": { "1002266": "1003991", "1002268": "1003998", "1002269": "1003999", "1002272": "1003994", "1002273": "1003997", "1002274": "1003995", "1002412": "1004017", "1003099": "1011542", "1003923": "1002356", "1003929": "1002355", "1003933": "1002366", "1003935": "1002368", "1003945": "1003331", "1003966": "1002836", "1003967": "1002837", "1003968": "1002838", "1003969": "1003327", "1003980": "1003990", "1003986": "1003992", "1005625": "1002359" }, "APPLY_SUBTABLE_FILTERS": false, "APPLY_SUBTABLE_GROUPING": false }
    ), response);
    if (result) return result;
}//END_RAGIC_AUTOGEN_CR_72440


function _RAGIC_AUTOGEN_CR_76725(nodeId) {
    // WARNING: Please DO NOT edit auto-generated script.
    var result = db.entryCopier(JSON.stringify(
{ "THIS_PATH": "/forms9/7", "THIS_NODEID": nodeId, "NEW_PATH": "/forms9/3", "CHECK_MUST_FIELDS": false, "CHECK_VALIDATION_FIELDS": false, "CHECK_ALL_MUST_FIELDS_MAPPED": false, "SILENT": false, "RECAL_ALL_FORMULAS": true, "EXEC_WORKFLOW": true, "LOCK_SOURCE": false, "COPY": { "1002359": "1003115", "1002366": "1008201", "1002400": "1003122", "1002836": "1003131", "1002838": "1007548", "1003991": "1003124", "1003994": "1003127", "1003998": "1003125", "1004017": "1003133", "1007510": "1011006", "1010958": "1011003", "1010959": "1011005", "1012555": "1003123", "1012556": "1003124", "1012558": "1003127", "1012599": "1003133", "1012902": "1012907" }, "APPLY_SUBTABLE_FILTERS": false, "APPLY_SUBTABLE_GROUPING": false }
    ), response);
    if (result) return result;
}//END_RAGIC_AUTOGEN_CR_76725
