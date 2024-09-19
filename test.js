function doMoveMO(nodeId, type, enterPath) {
    var pathMaster = '/forms3/142';
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
        }
        */
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
            var path = '/forms3/30';//生產紀錄表
            var query = db.getAPIQuery(path);
            var strInfo = "";
            var jsBackup;
            switch (type) {
                case 4://執行還原子表格移MO
                    jsBackup = JSON.parse(entry.getFieldValue(1013249));
                    for (var i = 0; i < jsBackup.length; i++) {
                        //response.setMessage(JSON.stringify(jsBackup[i]));
                        var results = query.getAPIEntry(jsBackup[i]['ID']);
                        results.setFieldValue(1004407, jsBackup[i]['Before']);
                        results.setFieldValue(1007448, jsBackup[i]['Before']);
                        results.setFieldValue(1000500, "JS移動MO\r\n" + entry.getFieldValue(1013231));
                        results.save();
                    }
                    //entry.setFieldValue(1013249, JSON.stringify(jsBackup));
                    entry.save();
                    break;
                case 3://執行子表格移MO
                    jsBackup = [];
                    var numMove = 0;
                    for (var Subtable_i = 0, SubtableID = 1013247; Subtable_i < entry.getSubtableSize(SubtableID); Subtable_i++) {
                        if (entry.getSubtableFieldValue(SubtableID, Subtable_i, 1013245) != "") {
                            var js = {
                                'ID': entry.getSubtableFieldValue(SubtableID, Subtable_i, 1013248),
                                'KeyValue': entry.getSubtableFieldValue(SubtableID, Subtable_i, 1013256),
                                'Before': entry.getSubtableFieldValue(SubtableID, Subtable_i, 1013244),
                                'After': entry.getSubtableFieldValue(SubtableID, Subtable_i, 1013245),
                            }
                            jsBackup.push(js);
                            var results = query.getAPIEntry(js['ID']);
                            results.setFieldValue(1004407, js['After']);
                            results.setFieldValue(1007448, js['After']);
                            results.setFieldValue(1000500, "JS移動MO\r\n" + entry.getFieldValue(1013231));
                            results.save();
                            numMove++;
                        }
                    }
                    response.setMessage("子表格移動了:" + numMove + "筆");
                    entry.setFieldValue(1013249, JSON.stringify(jsBackup));
                    entry.save();
                    break;
                case 2://還原移動
                    jsBackup = JSON.parse(entry.getFieldValue(1013242));
                    for (var i = 0; i < jsBackup['ID'].length; i++) {
                        var entryTB19 = query.getAPIEntry(jsBackup['ID'][i]);
                        entryTB19.setFieldValue(1004407, jsBackup['Before'][i]);
                        entryTB19.setFieldValue(1007448, jsBackup['Before'][i]);
                        entryTB19.save();
                        strInfo += "[url=" + jsBackup['Link'][i] + ']' + jsBackup['ID'][i] + "=" + jsBackup['Before'][i] + "<" + jsBackup['After'] + "[/url]\r\n";
                    }
                    entry.setFieldValue(1013241, strInfo);
                    entry.save();
                    break;
                case 1://移動MO並讓子表格帶出箱號
                    {
                        jsBackup = {};
                        jsBackup['ID'] = [];
                        jsBackup['Link'] = [];
                        jsBackup['Before'] = [];
                        Formulas = [];
                        jsBackup['KeyValue'] = [];
                        jsBackup['After'] = entry.getFieldValue(1013230);
                        //[ ]定義各種篩選
                        {
                            //MO
                            var regexStr = entry.getFieldValue(1013229);
                            if (regexStr != "") {
                                query.addFilter(1004407, 'regex', regexStr);
                            }
                            var str1, str2;
                            //卷號1(箱號)
                            regexStr = entry.getFieldValue(1013226);
                            if (regexStr != "") {
                                var matchResult = regexStr.match(/\[(.*)\]/);
                                if (matchResult !== null) {
                                    str1 = matchResult[1];
                                    jsBackup['Roll1'] = str1;
                                }
                                //query.addFilter(1000495, 'regex', jsBackup['Roll1']);
                                query.addFilter(1000495, '=', regexStr);
                            }

                            //卷號2(進料流水號)
                            regexStr = entry.getFieldValue(1013227);
                            if (regexStr != "") {
                                var matchResult = regexStr.match(/(?:\s+)(\S*)(?:\s+)(\S*)/);
                                if (matchResult !== null) {
                                    str1 = matchResult[1];
                                    str2 = matchResult[2];
                                    jsBackup['Roll2'] = str1 + "\\s+" + str2;
                                }
                                //query.addFilter(1005326, 'regex', jsBackup['Roll2']);
                                query.addFilter(1005326, '=', regexStr);
                            }
                            //卷號3(進料流水號)
                            regexStr = entry.getFieldValue(1013228);
                            if (regexStr != "") {
                                var matchResult = regexStr.match(/(?:\s+)(\S*)(?:\s+)(\S*)/);
                                if (matchResult !== null) {
                                    str1 = matchResult[1];
                                    str2 = matchResult[2];
                                    jsBackup['Roll3'] = str1 + "\\s+" + str2;
                                }
                                //query.addFilter(1008405, 'regex', jsBackup['Roll3']);
                                query.addFilter(1008405, '=', regexStr);
                            }
                            //起始時間
                            regexStr = entry.getFieldValue(1013342);
                            if (regexStr != "") {
                                query.addFilter(1000501, '>=', regexStr);
                            }
                            //結束時間
                            regexStr = entry.getFieldValue(1013343);
                            if (regexStr != "") {
                                query.addFilter(1000501, '<=', regexStr);
                            }
                        }
                        var error_i = 0;
                        var results = query.getAPIResultList();
                        if (results.length == 0) {
                            response.setMessage("匹配不到!");
                            return;
                        }

                        //如果匹配不到=料號
                        if (jsBackup['Roll1'] == null || true) {

                            //[ ]將篩選後的資料變更MO並記錄
                            for (var i = 0; i < results.length; i++) {
                                error_i++;

                                if (error_i > 20) {
                                    response.setMessage("筆數超過20!結束執行");
                                    return;
                                }
                                jsBackup['ID'].push(results[i].getRootNodeId());
                                jsBackup['KeyValue'].push(results[i].getFieldValue(1008435));
                                jsBackup['Link'].push(_URL + path + '/' + jsBackup['ID'][i]);
                                jsBackup['Before'].push(results[i].getFieldValue(1004407));
                                //計算不重複的卷號1
                                if (Formulas.indexOf(results[i].getFieldValue(1000495)) < 0) {
                                    Formulas.push(results[i].getFieldValue(1000495));
                                }
                                // results[i].setFieldValue(1004407, jsBackup['After']);
                                //results[i].setFieldValue(1007448, jsBackup['After']);
                                //results[i].setFieldValue(1000500, "JS移動MO\r\n" + entry.getFieldValue(1013231));
                                //results[i].save();
                                strInfo += "[url=" + jsBackup['Link'][i] + ']' + jsBackup['KeyValue'][i] + "=" + jsBackup['Before'][i] + ">" + jsBackup['After'] + "[/url]\r\n";
                            }

                            //[ ]篩選不重複的卷號1後一一帶入子表格
                            var iSubtable = 0;
                            for (var varBox_i = 0; varBox_i < Formulas.length; varBox_i++) {
                                //偶爾會跳錯誤，過一下在執行
                                var query1 = db.getAPIQuery('/forms3/30');

                                regexStr = Formulas[varBox_i];
                                //regexStr = regexStr.match(/\[(.*)\]/)[1];                                                        
                                query1.addFilter(1000495, '=', regexStr);
                                results = query1.getAPIResultList();

                                for (var j = 0, SubtableVal = 1013247, numLine = iSubtable * -1 - 100; j < results.length; j++) {
                                    var indexNum = iSubtable + j < entry.getSubtableSize(SubtableVal) ? entry.getSubtableRootNodeId(SubtableVal, iSubtable + j) : numLine - j;
                                    entry.setSubtableFieldValue(1013248, indexNum, results[j].getRootNodeId());
                                    entry.setSubtableFieldValue(1013256, indexNum, results[j].getFieldValue(1008435));
                                    entry.setSubtableFieldValue(1013243, indexNum,
                                        "[url=" + _URL + path + '/' + results[j].getRootNodeId() + ']' + results[j].getFieldValue(1000495) + "[/url]");
                                    entry.setSubtableFieldValue(1013244, indexNum, results[j].getFieldValue(1004407));//原本MO
                                    //如果和原本MO一樣自動填入移到MO
                                    results[j].getFieldValue(1004407) == entry.getFieldValue(1013233) ?
                                        entry.setSubtableFieldValue(1013245, indexNum, entry.getFieldValue(1013230)) :
                                        entry.setSubtableFieldValue(1013245, indexNum, "");//移到MO
                                }
                                iSubtable += results.length;
                            }

                            for (var j = iSubtable, SubtableVal = 1013247, numLine = -100; j < entry.getSubtableSize(SubtableVal); j++, numLine--) {
                                entry.deleteSubtableRowByRowNumber(SubtableVal, j)
                            }

                            entry.setFieldValue(1013241, strInfo);
                            entry.setFieldValue(1013242, JSON.stringify(jsBackup));
                            entry.loadAllLinkAndLoad();
                            entry.save();
                        }
                        //[ ]篩選使用該箱號的紀錄帶進子表格
                        //HACK沒問題能刪除該條件    
                        else {
                            for (var i = 0; i < results.length; i++) {
                                if (results[i].getFieldValue(1005326) != "") {
                                    jsBackup['Roll2'] = results[i].getFieldValue(1005326);
                                    break;
                                }
                            }
                            //entry.setFieldValue(1013226, "");
                            entry.setFieldValue(1013227, jsBackup['Roll2']);
                            entry.loadAllLinkAndLoad();
                            entry.save();
                            doMoveMO(entry.getRootNodeId(), 1);
                            return;
                        }
                    }
                    break;

                //[ ]子表格帶入MO
                default:
                    if (entry.getFieldValue(1013261) != "") {
                        entry.loadAllLinkAndLoad();
                        var query = db.getAPIQuery('/forms10/27');
                        query.addFilter(1004404, 'regex', entry.getFieldValue(1013261));
                        var results = query.getAPIResultList();
                        for (var j = 0, SubtableVal = 1013253, numLine = -100; j < results.length; j++) {
                            if (j < entry.getSubtableSize(SubtableVal)) {
                                entry.setSubtableFieldValue(1013251, entry.getSubtableRootNodeId(SubtableVal, j), results[j].getFieldValue(1008230));
                            }
                            else {
                                entry.setSubtableFieldValue(1013251, numLine - j, results[j].getFieldValue(1008230));
                            }
                        }
                        for (var j = results.length, SubtableVal = 1013253, numLine = -100; j < entry.getSubtableSize(SubtableVal); j++, numLine--) {
                            entry.deleteSubtableRowByRowNumber(SubtableVal, j)
                        }
                        entry.loadAllLinkAndLoad();
                        entry.save();
                    }
            }
            //[ ]刷新相關MO

            switch (type) {
                case 0:
                    break;
                default:
                    var Formulas = [];
                    if (Formulas.indexOf(entry.getFieldValue(1013229)) < 0)
                        Formulas.push(entry.getFieldValue(1013229));
                    if (Formulas.indexOf(entry.getFieldValue(1013230)) < 0)
                        Formulas.push(entry.getFieldValue(1013230));

                    if (entry.getFieldValue(1013242) != "") {
                        var jsBackup = JSON.parse(entry.getFieldValue(1013242));
                        for (var i = 0; i < jsBackup['Before'].length; i++) {
                            if (Formulas.indexOf(jsBackup['Before'][i]) < 0)
                                Formulas.push(jsBackup['Before'][i]);
                        }
                        if (Formulas.indexOf(jsBackup['After']) < 0)
                            Formulas.push(jsBackup['After']);
                    }
                    //response.setMessage(JSON.stringify(jsBackup));

                    if (entry.getFieldValue(1013249) != "") {
                        var jsBackup = JSON.parse(entry.getFieldValue(1013249));
                        for (var i = 0; i < jsBackup.length; i++) {
                            if (Formulas.indexOf(jsBackup[i]['Before']) < 0)
                                Formulas.push(jsBackup[i]['Before']);
                            if (Formulas.indexOf(jsBackup[i]['After']) < 0)
                                Formulas.push(jsBackup[i]['After']);
                        }
                    }
                    //response.setMessage(JSON.stringify(Formulas));
                    //重算相關SO的子表單
                    for (var i = 0; i < Formulas.length; i++) {
                        if (Formulas[i] != "") {
                            query = db.getAPIQuery('/forms10/27');
                            query.addFilter(1008230, 'regex', Formulas[i]);
                            var entryFormulas = query.getAPIResultList();
                            //response.setMessage("Formulas:"+Formulas[i]);
                            entryFormulas[0].recalculateAllFormulas();
                            entryFormulas[0].save();
                        }
                    }

            }

        }
        queryTOresults();
    }
    response.setStatus('SUCCESS');
    response.setMessage('執行完成');
}
