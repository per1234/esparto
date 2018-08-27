var ws1;
var pins=[];
var lbls=[];
var dlys=[];
var cfgs=[];
var ephs=[];
var gh;
var gq;
var gp;
var ga;

class graph{
	decorate(){
		var incr=this.maxY/this.ticks;
		var c=this.ctx;
	
		c.fillStyle="#000";
		c.font = '8px sans-serif';	
		c.beginPath();
		for(var i=0;i<this.ticks-1;i++){
			var y=incr*(i+1);
			var plotY=this.norm(y);
			c.moveTo(0,plotY);
			c.lineTo(10,plotY);
			c.fillText(Math.round(y),12,plotY);	
		}
		c.stroke();
	}	
	
	drawFrame(){
		var c=this.ctx;
		c.fillStyle="#fff";
		c.fillRect(0,0,this.width,this.height);
		c.strokeStyle="#000";
		c.strokeRect(0,0,this.width,this.height);		
		this.decorate();
	}	
	constructor(ele,maxY,base,ticks,dp){
		console.log("GRF: ele="+ele+" maxY="+maxY+" base="+base+" ticks="+ticks+" dp="+dp);
		this.ele=ele;
		this.maxY=maxY;
		this.base=base;
		this.ticks=ticks;
		this.dp=dp;		
		var g=document.getElementById(ele);
		var c=this.ctx=g.getContext("2d");
		this.height=g.height;
		this.width=g.width;
		this.nPoints=this.width/base;
		this.points=[];
		for(var i=0;i<this.nPoints;i++)	this.points[i]=0;		
	}
	
	norm(v){
		var h=this.height;
		return h-((v/this.maxY)*h);
	}

	plot(v){
		this.points.shift();
		this.points.push(v);
		
		this.drawFrame();
		var x=0;
		var y=0;	
		this.sigma=0;
		var slide=parseInt(this.base);
		var c=this.ctx;
		c.strokeStyle="#00f";		
		
		c.beginPath();		
		for(var i=0;i<this.nPoints;i++){
			y=this.points[i];
			this.sigma+=parseInt(y);
			c.lineTo(x,this.norm(y));
			x+=slide;
		}
		c.stroke();
		
		var avg=this.sigma/this.nPoints;
		c.beginPath();		
			c.strokeStyle="#f00";
			c.setLineDash([5, 5]);
			c.moveTo(0,this.norm(avg));
			c.lineTo(this.width,this.norm(avg));
		c.stroke();
		
		c.setLineDash([]);
		c.fillStyle="#f00";
		c.font = '8px sans-serif';
		c.fillText(avg.toFixed(this.dp),this.width / 2 ,this.norm(avg) - 3);
	}				
}

var pinstyle=[
	"UNUSED",
	"OUTPUT",
	"RAW",
	"DEBOUNCED",
	"FILTERED",	
	"LATCHING",
	"RETRIGGERING",
	"ENCODER",
	"ENCODER_AUTO",
	"REPORTING",
	"TIMED",
	"POLLED",
	"INTERRUPT",
	"3-STAGE"
  ];

function labelPin(pin,v,t){
	lbls[pin].innerHTML=v;
	lbls[pin].setAttribute( "title", t);			
}
function definePin(msg){
	var labdata=msg.split(":");
	var labvalues=labdata[1].split(" ");
	var pin=parseInt(labdata[0]);
	var digitalPin=parseInt(labvalues[0]);
	var type=parseInt(labvalues[1]);
	var style=parseInt(labvalues[2]);
	pins[pin].setAttribute("title", pinstyle[style]);
	if(style) pins[pin].id="p"+pin;
	if(style==1){ setClick(pin); }
	if(pin < 18){
		switch(type){
			case 0:
				lbls[pin].innerHTML="XX";
				lbls[pin].setAttribute("style","visibility:hidden");
				pins[pin].setAttribute("style","visibility:hidden");
				pins[pin+18].setAttribute("style","visibility:hidden");
				break;
			case 1:
				labelPin(pin,"D"+digitalPin," MAY AFFECT BOOTING - USE CAUTION");
				break;
			case 2:
				labelPin(pin,"RX","CANNOT USE AT SAME TIME AS SERIAL");													
				break;
			case 3:
				labelPin(pin,"TX","CANNOT USE AT SAME TIME AS SERIAL");													
				break;
			case 4:
				labelPin(pin,"D"+digitalPin,"Standard GPIO pin");													
				break;
			case 5:
				labelPin(pin,"D"+digitalPin,"NONSTANDARD GPIO - MAY CONFLICT WITH WAKE/SLEEP");													
				break;
			case 6:
				labelPin(pin,"AD","ADC");													
				break;
			case 7:
				labelPin(pin,"LD","Builtin LED");	
				break;
			case 8:
				labelPin(pin,"BN","Builtin Button");	
				break;
			case 9:
				labelPin(pin,"RY","Builtin Relay");												
				break;
			default:
				labelPin(pin,digitalPin,"UNNAMED PIN "+digitalPin);
				break;
		}
	}
	else if(style < 3) pins[pin].setAttribute("style","visibility:hidden");
}
function changeDevice(evt){
	evt.preventDefault();
	var newname=document.getElementById("newname").value;
	var ssid=document.getElementById("ssid").value;
	var pw=document.getElementById("pw").value;
	ws1.send(newname+","+ssid+","+pw);
	return false;	
}
function setClick(i){
	pins[i].setAttribute("onclick","pinClick("+i+");");
	pins[i].setAttribute("class",pins[i].getAttribute("class")+" output");
}
function pinClick(p){ ws1.send(p); }

function vc(){	ws1.send("!"+this.name+"!"+this.value); }

function config(e){
	for(var i=0;i<ephs.length;i++) ephs[i].style.display=ephs[i].classList[2]==e.classList[1] ? "block":"none";
	ws1.send("*"+e.classList[1]);
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
	for(var i=0;i<dlys.length;i++){ // lazy load images
		setTimeout(function(x){
			var c=x.classList[1];
			x.src=c+".png";			
			},2500,dlys[i]);
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
				pins[i].setAttribute("class",state=="0" ? "pin ld led-red":"pin ld led-green");
			}
			else definePin(msg);
		} else decodeWsMsg(msg);
	};
	document.getElementById("wifi").style.display="block";
});