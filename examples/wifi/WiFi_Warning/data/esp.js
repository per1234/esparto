var ws1;
var pins=[];
var lbls=[];
var dlys=[];
var cfgs=[];
var ephs=[];
var ths=[];
var chx=[];
var gh;
var gq;
var gp;
var ga;
var gl;

var pinstyle=[
	"UNUSED",
	"RAW",
	"OUTPUT",
	"DEBOUNCED",
	"FILTERED",	
	"LATCHING",
	"RETRIGGERING",
	"ENCODER",
	"ENCODER_AUTO",
	"REPORTING",
	"TIMED",
	"POLLED",
  "DEFAULT_OUT",
  "STD3STG",
  "ENCODER_B",
  "3-STAGE"
  ];

function pinDetailsShort(type,digitalPin){
		switch(type){
			case 1:
			case 4:
			case 5:
				return "D"+digitalPin;
			case 2:
				return "RX";													
			case 3:
				return "TX";																					
			case 6:
				return "AD";													
			case 7:
				return "LD";	
			case 8:
				return "BN";	
			case 9:
				return "RY";												
			default:
				return "&nbsp;&nbsp;";	
		}
}

function pinDetails(pin,type,digitalPin){
		var rv="GPIO"+pin+" is "+pinDetailsShort(type,digitalPin);
		switch(type){
			case 1:
				return rv+" MAY AFFECT BOOTING - USE CAUTION";
			case 2:
				return rv+" CANNOT USE AT SAME TIME AS SERIAL";													
			case 3:
				return rv+" CANNOT USE AT SAME TIME AS SERIAL";													
			case 4:
				return rv+" Standard GPIO pin";													
			case 5:
				return rv+" NONSTANDARD GPIO - MAY CONFLICT WITH WAKE/SLEEP";													
			case 6:
				return rv+" Analog to Digitial Converter";													
			case 7:
				return rv+" Builtin LED";	
			case 8:
				return rv+" Builtin Button";	
			case 9:
				return rv+" Builtin Relay";												
			default:
				return rv+" UNNAMED PIN "+digitalPin;
		}	
}

function definePin(msg){
	var labdata=msg.split(":");
	var labvalues=labdata[1].split(" ");
	var pin=parseInt(labdata[0]);
	var digitalPin=parseInt(labvalues[0]);
	var type=parseInt(labvalues[1]);
	if(type){
		var style=parseInt(labvalues[2]);
		pins[pin].setAttribute("style","visibility:visible");
		pins[pin].setAttribute("title", pinstyle[style]);
		if(style) pins[pin].id="p"+pin;
		if(pin < 18){
      if(style==2 || style==12){ setClick(pin); }
			lbls[pin].innerHTML=pinDetailsShort(type,digitalPin);
			lbls[pin].setAttribute( "title", pinDetails(pin,type,digitalPin));			
		}
    else {
      console.warn("COOKED ",pin," style=",style," type=",type);
      if(style < 2) pins[pin].setAttribute("style","visibility:hidden");
    }
	}	else ths[pin].setAttribute("class","dpu");
}

function changeDevice(evt){
	evt.preventDefault();
	var newname=document.getElementById("newname").value;
	var lex=document.getElementById("lexname").value;
	var ssid=document.getElementById("ssid").value;
	var pw=document.getElementById("pw").value;
	ws1.send("cd,"+newname+","+lex+","+ssid+","+pw);
	return false;	
}

function spoolChange(moi){ ws1.send("^"+moi.value); }

function ssnull(){ if(chx[0].checked) for(var i=1;i<6;i++) chx[i].checked=false; }

function ssother(moi){  if(moi.checked) chx[0].checked=false;  }

function spool(evt){
	evt.preventDefault();
  var ss="sp,"+document.getElementById("ssrc").value;
  for(var i=0;i<chx.length;i++) ss+="/"+((chx[i].checked) ? "1":"0"); 
  ws1.send(ss);
	return false;	
}

function dynamicPin(evt){
	evt.preventDefault();
	var f=evt.target;
	var fields=Array.from(f.elements).filter(e => e.getAttribute("name"));
	var ss="dp,";
	for(var i=0;i<fields.length-2;i++) {
    var value;
		if(fields[i].type=="radio") {
      value=fields[i].checked ? "1":"0";
      i++;
    } else value=fields[i].value;
    ss+=(value.replace(/\//g,"#")+"/");
	}
  ws1.send(ss.replace(/.$/,","+(fields[i].checked ? "1":"0")));
  
  var ssel=document.getElementById("selstyle");
  ssel.value=0;
  dpstyleChange(ssel);
  document.getElementById("fsp").setAttribute("disabled","");
  
  var sax=document.getElementById("axion");
  sax.value=0;
  dpAxionChange(sax);
   
	document.getElementById("dpsel").value="click red pin #";
	return false;
}

function dynpSelect(i){
	document.getElementById("dpsel").value=i;
	document.getElementById('fsp').removeAttribute("disabled");
}

function dynpEnter(i){ ws1.send("."+i);	}

function gpAddField(name,ipf,hangoff){
	var lbl=document.createElement("label");
	lbl.setAttribute("for",name);
	lbl.innerHTML=name;
	var field=document.createElement("div");
	field.className="fel";
	field.appendChild(lbl);
	field.appendChild(ipf);
	document.getElementById(hangoff).appendChild(field);
}

function gpSelect(data,id){
	var opts=data.opts;
	var sel=document.createElement("select");
	sel.name=data.name;
	sel.id=data.name;
	sel.required="required";
	sel.setAttribute("style","margin-left: 4px;"); // WTF????????
	var selopt;	
	for(var i=0;i<opts.length;i++){
		selopt=document.createElement("option");
		selopt.value=opts[i].value;
		selopt.innerHTML=opts[i].disp;
		sel.appendChild(selopt);	
	}
	gpAddField(data.name,sel,id);
}

function dpscMode(){
	gpSelect({name: "Mode",
						opts:	[
									{value: 0, disp: "INPUT" },
									{value: 2, disp: "INPUT_PULLUP" }
									]
						},
		"dpextra");
}

function dpscPinB(exclude){
	var data={
		name: "PinB",
		opts:[]
	};
	var dph=document.getElementsByClassName("dph");
	for(var i=0;i<dph.length;i++){
		if(dph[i].tooltip!="CAN'T USE"){
			var pin=parseInt(dph[i].innerHTML);
			if(pin!=exclude && pin!=17)	data.opts.push({value: pin, disp: "GPIO "+pin });
		}
	}
	gpSelect(data,"dpextra");		
}

function axOutputs(fld){
	var data={
		name: fld,
		opts:[]
	};
	var ops=document.getElementsByClassName("output");

	for(var i=0;i<ops.length;i++){
		var pin=parseInt(ops[i].id.replace(/p/,""));
		if(pin<18) data.opts.push({value: pin, disp: "GPIO "+pin });
	}
	gpSelect(data,"axdata");
}

function makeRadio(name,value){
	var r=document.createElement("input");
	r.type="radio";
	r.name=name;
	r.id=name+value;
	r.value=value;
	if(value=="0") r.setAttribute("checked","");
	return r;
}

function binaryRadio(name,one,zero,hangoff){
	var s1=document.createElement("span");
	s1.innerHTML=one;
	
	var s2=document.createElement("span");
	s2.innerHTML=zero;
	
	var c=document.createElement("div");
	c.className="dhl";
	c.appendChild(s1);
	c.appendChild(makeRadio(name,"1"));	
	c.appendChild(s2);
	c.appendChild(makeRadio(name,"0"));
	
	gpAddField(name,c,hangoff);
}

function dpscValue(field,id="dpextra",num=true,mand=true){
	var ipv=document.createElement("input");
	ipv.name=field;
	ipv.id=field;
	ipv.type=num ? "number":"text";
	if(mand) ipv.setAttribute("required","");
	ipv.setAttribute("checked","");
	ipv.setAttribute("style","margin-left: 4px;"); // WTF????????	
	gpAddField(field,ipv,id);
}

function ajaxList(c,n){
  gpSelect({name: n, opts:[]},"axdata");
  ws1.send(c+n);  
}

function dpAxionChange(i){
	document.getElementById("axdata").innerHTML="";
	var choice=parseInt(i.value);
	switch(choice){
		case 0:
		case 1:
		case 2:
			break;
		case 3:
			axOutputs("PassTo");
      binaryRadio("Invert","Yes","No","axdata");
			break;
    case 4:
    case 6:
    case 7:
    case 9:
    case 10:
    case 11:
      ajaxList("~","Var");
      break;
    case 5:
      ajaxList("~","Var");
      dpscValue("Value","axdata",false);
      binaryRadio("Txt_Num","TXT","NUM","axdata");
      break;
    case 8:
      ajaxList("+","Cmd");     
      dpscValue("Payload","axdata",false,false);
      break;
    case 12:
      axOutputs("Pin");
      dpscValue("Rate","axdata");
      break;
    case 13:
      axOutputs("PWM_Pin");
      dpscValue("Period","axdata");
      dpscValue("Duty Cycle","axdata");
      break;
    case 14:
      axOutputs("Pattern_Pin");
      dpscValue("Timebase","axdata");
      dpscValue("Pattern","axdata",false);
      break;
    case 15:
      axOutputs();
      break;
		default:
			console.warn("odd choice...",choice);
	}
	if(choice) document.getElementById('fsk').removeAttribute("disabled");
	else document.getElementById('fsk').setAttribute("disabled","");
}
function dummyMode(dpe){
  var hid=document.createElement("input");
  hid.type="hidden";
  hid.value="1";
  hid.name="hm";
  hid.setAttribute("checked","checked");
  dpe.appendChild(hid);
}
function dpstyleChange(moi){
	var pin=document.getElementById("dpsel").value;
	var choice=parseInt(moi.value);
	document.getElementById("axdata").innerHTML="";
	var dpe=document.getElementById("dpextra");
	dpe.innerHTML="";
	switch(choice){
		case 2: // output
		case 12: // dfltout
      dummyMode(dpe);
			binaryRadio("Active","Hi","Lo&nbsp;","dpextra");
			binaryRadio("Initial","On","Off","dpextra");				
			break;
		case 1: // raw
			dpscMode();
			break;
		case 3: // debounced
		case 5:	// latching
		case 10: // timed
			dpscMode();
			dpscValue("Debounce");
			break;			
		case 9: // reporting
			dpscMode();
			dpscValue("Debounce");
			dpscValue("Freq'y");
			break;
		case 4: // filter
			dpscMode();
			binaryRadio("Filter","Hi","Lo","dpextra");
			break;
		case 6: // retrig		
			dpscMode();
			dpscValue("Timeout");
			binaryRadio("Active","Hi","Lo","dpextra");			
			break;
		case 7: // encoder
			dpscMode();
			dpscPinB(pin);
			break;
		case 8: // encoder auto
			dpscMode();
			dpscPinB(pin);
			dpscValue("Vmin");
			dpscValue("Vmax");
			dpscValue("Vinc");
			dpscValue("Vset");
			break;
		case 11: // polled
			dpscMode();
			dpscValue("Freq'y");
			binaryRadio("A-D","ADC","Digital","dpextra");
			break;
    case 13:
      console.warn("STD 3-STAGE ",choice);
      dummyMode(dpe);
      dpscValue("Debounce");
      break;
		default:
			console.warn("what choice? ",choice);
			break;
	}
	if(choice) {
		document.getElementById('fsa').removeAttribute("disabled");
		document.getElementById("axion").setAttribute("onchange","dpAxionChange(this);");
	}	else document.getElementById('fsa').setAttribute("disabled","");
}

function addDynpHandlers(){
	for(var i=0;i<lbls.length;i++){
		if(lbls[i].innerHTML=="&nbsp;&nbsp;"){
			if(ths[i].getAttribute("class")!="dpu"){
				ths[i].setAttribute("onmouseenter","dynpEnter("+i+");");
				ths[i].setAttribute("class","dph");				
			}
		} else ths[i].setAttribute("onclick","killclick("+i+");");     
	}
}

function killclick(i){ if(confirm("Do you really want to delete GPIO "+i)) ws1.send("-"+i); }

function loseDynpHandler(i){
  ths[i].removeAttribute("oncontextmenu");
  ths[i].removeAttribute("onmouseenter");
  ths[i].removeAttribute("onclick");
  if(ths[i].getAttribute("class")!="dpu") ths[i].removeAttribute("class");
  ths[i].removeAttribute("tooltip");		
}

function loseDynpHandlers(){ for(var i=0;i<ths.length;i++) loseDynpHandler(i); }

function setClick(i){
	pins[i].setAttribute("onclick","pinClick("+i+");");
	pins[i].setAttribute("class",pins[i].getAttribute("class")+" output");
}

function pinClick(p){ ws1.send(p); }

function vc(){	ws1.send("!"+this.name+"!"+this.value); }

function config(e){
	for(var i=0;i<ephs.length;i++) ephs[i].style.display=ephs[i].classList[2]==e.classList[1] ? "block":"none";
	if(e.title=="Pins") addDynpHandlers();
	else loseDynpHandlers();
	ws1.send("*"+e.classList[1]);
}

function killPin(i){
  pins[i].removeAttribute("style");
  pins[i].removeAttribute("title");
  pins[i].removeAttribute("id");
}

function bootfn(){ ws1.send(40); }

function factfn(){ ws1.send(64); }

function homefn(){ document.location="/"; }

function selchange(e){ document.getElementById("invoke").value=e.target.value; }

function doit(){ ws1.send("?"+document.getElementById("invoke").value+"|"+document.getElementById("payload").value); }

document.addEventListener("DOMContentLoaded", function() {
	pins=document.getElementsByClassName("pin");
	lbls=document.getElementsByClassName("lbl");
	dlys=document.getElementsByClassName("hand");
	chx=document.getElementsByClassName("chx");
	ths=document.getElementsByTagName("th");
	for(var i=0;i<dlys.length;i++){ // lazy load images
		setTimeout(function(x){
			var c=x.classList[1];
			x.src=c+".png";			
			},750,dlys[i]);
	}
	cfgs=document.getElementsByClassName("cfg");
	for(i=0;i<cfgs.length;i++){ cfgs[i].setAttribute("onclick","config(this);"); }
	ephs=document.getElementsByClassName("eph");

	if(document.getElementById("root").value!=="/ws.htm"){	document.getElementById("home").style.visibility="visible";	}
	
	ws1 = new WebSocket("ws://"+document.getElementById("IP").innerHTML+"/ws");
	ws1.onerror = function(evt){ console.log(evt); };
	ws1.onclose = function(evt){ console.log(evt); };
	ws1.onmessage = function (evt) {
		var msg=evt.data;
		if(/^[0-9]/.test(msg[0])) {		
			if(msg.indexOf(",")!=-1){
				var pindata=msg.split(",");
				var i=parseInt(pindata[0]);
				var state=parseInt(pindata[1]);
        pins[i].classList.add(state=="0" ? "led-red":"led-green");
        pins[i].classList.remove(state=="0" ? "led-green":"led-red");
			}
			else {
				if(msg.indexOf(".")!=-1){
					var pindata=msg.split(".");
					var i=parseInt(pindata[0]);
					var j=parseInt(pindata[1]);
					var k=parseInt(pindata[2]);
					var legend;
					if(k){
						ths[i].setAttribute("onclick","dynpSelect("+i+");");
						legend=pinDetails(i,k,j);
					}
					else legend="CAN'T USE";
					ths[i].setAttribute("tooltip",legend);
				}
				else definePin(msg);
			}
		} else decodeWsMsg(msg);
	};
	document.getElementById("wifi").style.display="block";
});