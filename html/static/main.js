var layer = layui.layer;
var form = layui.form;
var table = layui.table;
var $ = layui.$;
var client_width = document.documentElement.clientWidth;
var client_height = document.documentElement.clientHeight;

// �Ƿ�װ��dll����
var g_install_dll = false;
// ����
var g_setting = []; 
// �򿪲˵�����������
var g_open_menu_index = -1;
// �Ƿ��ѿ�������CALL
var g_open_talk = false;
// c++�����
var cpp_obj = null, _c = null;

//layer.alert("�������.", { icon: 1 });
//ShowMsg("msg", null, 2);

function test(v, v2) {
    //alert(v+v2);
}

$(document).ready(function () {
    //ShowMsg(CallCpp(2, 3));
    $("#full").css("height", client_height + "px");
    setTimeout(function () {
        if (!cpp_obj) {
            fff();
            if (0)
                ShowMsg('�޷���ȡ��c++�����, ��������޷�����, �����´򿪱�����.', '��ʾ', 2);
        }
            
    }, 500);
});

function fff() {
    var zoom = detectZoom();
    if (client_height < $("body").height()) {
        var b_height = $("body").height();
        var bl = (b_height - client_height) / b_height * 100;
        bl = bl.toFixed(2);
        var msg = "ҳ�����δ��ʾ��ȫ(��ǰҳ��߶�" + b_height + ", ��������߶�" + client_height+", ���ɼ�����"+bl+"%),";
        msg += "��ǰ���ű���(" + zoom + "%), �볢�԰�(Ctrl+������)����ҳ��.";
        alert(msg);
    }
    $("body").on("mousedown", function (event) {
        event = event || window.event;
        if (cpp_obj && event.button == 2) {
            document.oncontextmenu = function (event) {
                if (typeof event.returnValue !== 'undefine')
                    event.returnValue = false;
                if (typeof event.preventDefault !== 'undefine')
                    event.preventDefault();
            }
        }
    });

    $("main").click(function () {
        if (parseInt($("#log").data('visable')) == 1) {
            ShowLog('log', 'r_tool', 'log_ul', 1);
        }

        $("#table_1_menu").fadeOut(500);
        g_open_menu_index = -1;
    });

    $(".tips").on("mouseover", function (event) {
        event = event || window.event;
        var left = $(this).offset().left;
        var top = $(this).offset().top;
        
        //alert(height);
        
        $("#tips_box").html($(this).data("text"));

        var width = $("#tips_box").width();
        var height = $(this).height();
        if ((left + width) >= client_width) {
            left = client_width - width - 30;
        }

        $("#tips_box").css({ "left": left, "top": event.pageY - 45 });
        $("#tips_box").show();
        

    });
    $(".tips").on("mouseout", function () {
        $("#tips_box").fadeOut(200);
    });

    $(".setting_radio input").on("change", function () {
        CallCpp("put_setting", $(this).attr("name"), parseInt($(this).val()), 0);
    });

    $(".my_ck_box").on("click", function () {
        var box = $(this).children("i");
        var text = box.text();

        var checked = 0;
        if (text.length) {
            box.html("");
            checked = 0;
        }
        else {
            box.html("&#xe605;");
            checked = 1;
        }

        CallCpp("put_setting", box.data("key"), checked, 0);
    });

    $(".menu li").on("mouseover", function () {
        if (!$(this).hasClass("disabled"))
            $(this).css("background", "#eeeeee");
    });
    $(".menu li").on("mouseout", function () {
        if (!$(this).hasClass("disabled"))
            $(this).css("background", "#ffffff");
    });
    $(".menu li").click(function () {
        if (!$(this).hasClass("disabled")) {
            var parent_id = $(this).parent("ul").attr("id");
            //CallCpp("select_menu", parent_id, g_open_menu_index, parseInt($(this).index()));
            var index = parseInt($(this).index());
            if (index == 0) {
                if (!g_install_dll && !Start())
                    return;

                CallCpp("open_game", g_open_menu_index);
            }
            if (index == 1) {
                CallCpp("close_game", g_open_menu_index);
            }
            if (index == 2) {
                CallCpp("in_team", g_open_menu_index);
            }

            $(this).parent("ul").fadeOut(300);
            //g_open_menu_index = -1;
        }
    });

    
    $("#table_1 tr").on("mouseover", function () {
        $(this).css("background", "#eeeeee");
    });
    $("#table_1 tr").on("mouseout", function () {
        $(this).css("background", "#ffffff");
    });
    $("#table_1 tr").on("mousedown", function (event) {
        event = event || window.event;
        if (event.button == 2) {
            if (!$(this).children("td:eq(0)").html()) {
                $("#table_1_menu").fadeOut(500);
                return;
            }

            g_open_menu_index = parseInt($(this).index());
            if (CallCpp("open_menu", parseInt($(this).index()))) {
                SetMenuDisabled("table_1_menu", 0, 1);
                SetMenuDisabled("table_1_menu", 1, 0);
                SetMenuDisabled("table_1_menu", 2, 0);
            }
            else {
                SetMenuDisabled("table_1_menu", 0, 0);
                SetMenuDisabled("table_1_menu", 1, 1);
                SetMenuDisabled("table_1_menu", 2, 1);
            }

            $("#table_1_menu").css({ "margin-left": (event.clientX + 10) + "px", "margin-top": (event.clientY + 5) + "px" });
            $("#table_1_menu").fadeIn(500);
        }
    });
}

function detectZoom() {
    var ratio = 0,
        screen = window.screen,
        ua = navigator.userAgent.toLowerCase();

    if (window.devicePixelRatio !== undefined) {
        ratio = window.devicePixelRatio;
    }
    else if (~ua.indexOf('msie')) {
        if (screen.deviceXDPI && screen.logicalXDPI) {
            ratio = screen.deviceXDPI / screen.logicalXDPI;
        }
    }
    else if (window.outerWidth !== undefined && window.innerWidth !== undefined) {
        ratio = window.outerWidth / window.innerWidth;
    }

    if (ratio) {
        ratio = Math.round(ratio * 100);
    }

    return ratio;
};

// ����
function ShowMsg(text, t, _icon) {
    if (!t)
        t = '��Ϣ';

    layer.open({
        icon: _icon,
        title: t,
        content: text,
        yes: function (index) {
            if (text.indexOf("���ȼ���") > -1)
                VerifyCard();
            layer.close(index);
        }
    }); 
}

// ��ʾ������
function ShowTool(id, sub_top) {
    return;
    var top = client_height - (client_height / 3);
    $("#" + id).show();
    $("#" + id).css({ "display": "block", "margin-top": (top - sub_top) + "px" });
}

// ����Ԫ�ؿ��Ϊ���ӿ�Ⱥ����ұ�����
function SetMarginLeftClientWidth(id) {
    $("#" + id).css({ "width": (client_width - 60) + "px", "margin-left": (client_width + 20) + "px" });
}

// ����Ԫ���붥������Ϊ��������߶�
function SetMarginTopClientHeight(id, sub_top) {
    $("#" + id).css({ "margin-top": (client_height - sub_top) + "px" });
}

// ����Ԫ��Ϊ���Ӹ߶�
function SetClientHeight(id, sub_top) {
    $("#" + id).css("height", client_height - sub_top);
}

// ����class
function SetClass(id, cla, rm_cla) {
    if (rm_cla)
        $("#" + id).removeClass(rm_cla);
    if (cla)
        $("#" + id).addClass(cla);
}

// ����Ԫ������
function SetText(id, text) {
    $("#" + id).html(text);
    if (id == 'fb_count') {
        $('#fb_count_log').html(text);
    }
}

// �˵���������
function SetMenuDisabled(id, row, v) {
    if (v)
        $("#" + id).children("li:eq(" + row + ")").addClass("disabled");
    else
        $("#" + id).children("li:eq("+row+")").removeClass("disabled");
}

// ��ť����
function SetBtnDisabled(id, v, text) {
    if (v)
        $("#" + id).addClass("layui-btn-disabled");
    else
        $("#" + id).removeClass("layui-btn-disabled");

    if (text)
        $("#" + id).html(text);
}

// ���һ�б��
function AddTableRow(id, col_count, key, v) {
    if (!col_count)
        col_count = $("#" + id).children("colgroup").children("col").length;

    var length = $("#" + id).children("tbody").children("tr").length;
    var text = "<tr data-index=" + length + ">";
    for (var i = 0; i < col_count; i++) {
        text += "<td>";
        if (i == 0 && key) {
            text += key;
        }
        if (i > 0 && v) {
            text += v;
        }
        text += "</td>";
    }
    text += "</tr>";

    $("#" + id).children("tbody").append(text);
}

// �Զ���������Դﵽ����
function FillTableRow(id, row_count, col_count) {
    if (!id)
        id = "table_1";
    if (!row_count) {
        var table_height = $("#" + id).height();
        var tr_height = table_height ? $("#" + id).children("tbody").find("tr:eq(0)").height() : 30;
        var div_height_p = $("#" + id).parent("div").height();
        row_count = parseInt((div_height_p - table_height) / tr_height); 
    }
    if (!col_count)
        col_count = $("#" + id).children("colgroup").children("col").length;

    for (var i = 0; i < row_count; i++) {
        AddTableRow(id, col_count, 0);
    }
}

// ���±���б�����
function UpdateTableText(id, row, col, text) {
    $("#"+id).children("tbody").find("tr:eq("+row+")").find("td:eq("+col+")").html(text);
}

// ���±��class
function UpdateTableClass(id, row, col, cla, add) {
    var obj = $("#" + id).children("tbody").find("tr:eq(" + row + ")");
    if (col > -1)
        obj = obj.find("td:eq(" + col + ")");

    if (add)
        obj.addClass(cla);
    else
        obj.removeClass(cla);
}

// ���±����ʽ
function UpdateTableStyle(id, row, col, key, value) {
    var obj = $("#" + id).children("tbody").find("tr:eq(" + row + ")");
    if (col > -1)
        obj = obj.find("td:eq(" + col + ")");

    obj.css(key, value);
}

var log_num = 0;
// ����ռ�
function AddLog(id, str, cla) {
    if (!id)
        id = 'log_ul';

    var first_enter = str[0] == "\n" ? true : false;
    var end_enter = str[str.length - 1] == "\n" ? true : false;

    str = str.replace("\n", "");

    var text = '';
    if (first_enter) text += '<li>----------------------------------------------------------------</li>';

    text += "<li>";
    if (cla)
        text = '<li class="' + cla + '">';

    log_num++;

    var d = new Date();
    text += "<span style='float:right;'>[" + AddZero(d.getMonth() + 1) + "-" + AddZero(d.getDate()) + " ";
    text += AddZero(d.getHours()) + ":" + AddZero(d.getMinutes()) + ":" + AddZero(d.getSeconds());
    text += "]</span>";
    text += "<span>" + log_num + ".</span> ";
    text += str;
    text += "</li>";

    if (end_enter) text += '<li>----------------------------------------------------------------</li>';

    $('#' + id).append(text);
    if (log_num > 1000) {
        $('#' + id).children("li:eq(0)").remove();
    }

    $('#log_ul').scrollTop($('#log_ul')[0].scrollHeight);
}

// ��ʾ�ռ�Ԫ��
function ShowLog(id, text_id, log_id, hide) {
    if (!id)
        id = "#log";
    else
        id = '#' + id;

    var text = '';
    if (!hide && parseInt($(id).data('visable')) == 0) {
        text = '��<br />��<br /><i class="layui-icon">&#xe603;</i><br />��<br />��<br />';
        $(id).data('visable', 1);
        //$(id).css("display", 'block');
        $(id).fadeIn(500);
        //$(id).animate({ "width":700 }, 100);

        $('#' + log_id).scrollTop($('#' + log_id)[0].scrollHeight);
    }
    else {
        text = '��<br />ʾ<br /><i class="layui-icon">&#xe602;</i><br />��<br />��<br />';
        $(id).data('visable', 0);
       // $(id).css("display", 'none');
        $(id).fadeOut(500);
        //$(id).animate({ "width": 0 }, 150);
    }

    $('#' + text_id).html(text);
}

// ����״̬����
function UpdateStatusText(text, flag) {
    $("#status").show();

    var icon = '';
    if (flag == 1)
        icon = 'xe645';
    else if (flag == 2) // ��
        icon = 'xe605';
    else if (flag == 3) // ��
        icon = 'x1006';
    else if (flag == 4) // ����
        icon = 'xe63d';
    else if (flag == 5) // ˮ��
        icon = 'xe636';

    if (icon)
        icon = '<i class="layui-icon">&#' + icon + ';</i> ';

    var d = new Date();
    text += " <span class=' cb'>[" + (d.getFullYear()) + "-" + AddZero(d.getMonth() + 1) + "-" + AddZero(d.getDate()) + " ";
    text += AddZero(d.getHours()) + ":" + AddZero(d.getMinutes()) + ":" + AddZero(d.getSeconds());
    text += "] </span>";
    $('#status_text').html(icon+text);
}

// ��ʾ���� 
function ShowSetting(id, h) {
    ShowSettingCallBack();
    $("#full").show();
    $("#" + id).css("margin-top", ((client_height - h) / 2 - 50)+"px");
    $("#" + id).show();
}

// ȷ������
function SaveSetting(id) {
    PutSetting(id);
    UpdateStatusText("���ø��³ɹ�.", 2);
    ShowMsg("���óɹ�.", "��ʾ", 1);

    if (id == "talk_setting") {
        if (g_setting['talk_open'] && !g_open_talk) {
            AutoTalk();
        }
    } 

    HideSetting();
}

// ��������
function HideSetting() {
    $(".setting_div").hide();
    $("#full").hide();
}

// �Զ�����CALL
function AutoTalk() {
    if (!g_setting['talk_open'] || !g_setting['talk_content']) {
        g_open_talk = false;
        return;
    }

    g_open_talk = true;
    //AddLog("log_ul", '����:'+g_setting['talk_content'], "blue");
    cpp_obj.Call("talk", g_setting['talk_content'], g_setting['talk_type']);
    setTimeout(AutoTalk, g_setting['talk_intval'] * 1000);
}

function ShowSettingCallBack() {
    //setTimeout(function () {
    for (var key in g_setting) {
        SetSetting(key, g_setting[key], true);
    }
    //}, 20);
}

// �������
function SetSetting(name, v, show) {
    if (typeof show === 'undefined' || !show) {
        g_setting[name] = v;
        return;
    }
    
    var obj = $(".setting_div input[name=" + name + "]");
    if (!obj.length) {
        obj = $(".setting_div select[name=" + name + "]");
        if (!obj.length)
            return;
    }
    if (obj[0].type == 'checkbox') {
        obj.attr('checked', v ? true : false);
    }
    else if (obj[0] == 'select') {
        $(obj).children('option').each(function () {
            if (this.value == v)
                this.selected = true;
        });
    }
    else {
        obj.val(v);
    }
}

// ��������
function PutSetting(id) {
    $('#'+id+' input').each(function () {
        var value = $(this).data("string") ? this.value : parseInt(this.value);
        if (this.type == 'checkbox') {
            value = this.checked ? 1 : 0;
        }
        SetSetting(this.name, value);

        CallCpp("put_setting", this.name, value, parseInt($(this).data("string")));
    });
    $('#' + id +' select').each(function () {
        var value = $(this).data("string") ? this.value : parseInt(this.value);
        SetSetting(this.name, value);

        CallCpp("put_setting", this.name, value, parseInt($(this).data("string")));
    });
}

// ����
function VerifyCard(obj) {
    var value = $('#card_input').val();
    if (value) {
        SetBtnDisabled("card_btn", 1, "ȷ����...");
        CallCpp("verify_card", value);
        SetBtnDisabled("card_btn", 0, "ȷ��");
    }
    HideSetting(); 
}

// ����
function UpdateVer(obj) {
    if ($('#update_btn').hasClass('update_btn_ing'))
        return;
        
    $('#update_btn').removeClass('update_btn');
    $('#update_btn').addClass("update_btn_ing");
    $('#update_btn').html('������...');

    cpp_obj.Call("update_ver");
}

// �������
function UpdateVerOk() {
    $('#update_btn').removeClass('update_btn_ing');
    $('#update_btn').addClass("update_btn");
    $('#update_btn').html('������<i class="layui-icon">&#xe607;</i>');
    UpdateStatusText("�������.", 2);
}

// ǰ�������
function AddZero(v) {
    if (parseInt(v) < 10)
        v = '0' + v;
    return v;
}

// ����c++�����
function SetCppObj(obj) {
    cpp_obj = obj;
    _c = cpp_obj;
    fff();
}

function Start() {
    g_install_dll = false;
    var result = CallCpp('start');
    if (result === -1) {
        ShowMsg("���ȼ���.", "��ʾ", 2);
        return false;
    }
    if (result === 0) {
        ShowMsg("����ʧ��, ԭ���뿴�ռ�.", "��ʾ", 2);
        return false;
    }

    if ($('#start_btn').html() == '����')
        $('#start_btn').html('ֹͣ');
    else
        $('#start_btn').html('����');

    g_install_dll = true;
    return true;
}

function AutoPlay() {
    if (!Start())
        return;

    var index;
    if ($('#auto_play_btn').html() == '�Զ��Ǻ�') {
        $('#auto_play_btn').html('ֹͣ�Ǻ�');
        index = -1;
    }
    else {
        $('#auto_play_btn').html('�Զ��Ǻ�');
        index = -2;
    }
    CallCpp("open_game", index, 0);
}
