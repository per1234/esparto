var grow=0;

function doForWsItems(p,fn){
	var owner=document.getElementById(p[1]);
	var n=(p.length-2)/2;
	for(i=0;i<n;i++) fn(owner,p[(i*2)+2],p[(i*2)+3]);
}

function decodeWsMsg(msg){
	var p=msg.split("|");
	switch(p[0]){
		case 'grf': // hate this FIXXXXXXXXXXXXXXX
			console.warn("LOAD GRAPH ",msg);
			switch(p[1]){
				case 'ga':
					ga=new graph(p[1],p[2],p[3],p[4],p[5]);
					break;
				case 'gh':
					gh=new graph(p[1],p[2],p[3],p[4],p[5]);
					break;
				case 'gl':
					gl=new graph(p[1],p[2],p[3],p[4],p[5]);
					break;
				case 'gq':
					gq=new graph(p[1],p[2],p[3],p[4],p[5]);
					break;
				case 'gp':
					gp=new graph(p[1],p[2],p[3],p[4],p[5]);
					break;
			}
			break;
		case 'adc':
			var tit=document.getElementById("titles").children[p[1]];
	        tit.classList.remove(p[3]);		
	        tit.classList.add(p[2]);		
			break;
		case 'aat':
			var c=document.getElementById(p[1]);
			var d=new Date();
			c.innerHTML=d.toLocaleString()+" "+p[2]+"\r\n"+c.innerHTML;
			break;
		case 'dpx':
			loseDynpHandler(parseInt(p[1]));
			break;
		case 'kil':
			i=parseInt(p[1]);
			killPin(i);
			killPin(i+18);
			lbls[i].removeAttribute("title");
			lbls[i].innerHTML="&nbsp;&nbsp;";			
			break;
		case 'beat':
			var onoff=parseInt(p[1]);
			document.getElementById("upt").innerHTML=p[2];
			document.documentElement.style.setProperty("--heart-color",onoff ? "#777777":"#ff0000");
			var heart=document.getElementById("hb");
			heart.setAttribute("class","cmd big");
			setTimeout(function(){ heart.setAttribute("class","cmd"); },250);
			if((grow%60)===0){
				var grass=document.getElementById("grass");
				var pc=parseFloat(window.getComputedStyle(grass).backgroundSize.replace(/%$/,''));
				if(pc!=100) pc+=0.25;
				else pc=0;
				grass.style.backgroundSize=pc+"%";
			}
			grow++;
			break;
		case 'vis':
			document.getElementById(p[1]).style.visibility="visible";
			break;
		case 'show':
			document.getElementById(p[1]).style.display="block";
			break;
		case 'tool':
			var o=document.getElementById(p[0]);
			var inp=document.createElement("input");
			inp.value=p[2];
			inp.name=p[1];
			inp.id=p[1];
			inp.onblur=vc;
			inp.required=true;
			var divlt=document.createElement("div");
			divlt.className="lt";
			divlt.appendChild(inp);
			var divdk=document.createElement("div");
			divdk.className="dk";
			divdk.innerHTML=p[1];
			var divsb=document.createElement("div");
			divsb.className="sb";										
			divsb.appendChild(divdk);
			divsb.appendChild(divlt);				
			o.appendChild(divsb);
			break;
		case 'aso': // add select options
			doForWsItems(p,function(o,v1,v2){	
				var selopt=document.createElement("option");
				selopt.value=v1;
				selopt.innerHTML=v2;
				o.appendChild(selopt);
			});
			break;
		case 'sso':
			var selem=document.getElementById(p[1]);
			var selopts=selem.childNodes;
			for(i=0;i<selopts.length;i++){
				if(selopts[i].value==p[2]){
					selopts[i].selected=true;
					break;
				}
			}
			break;
		case 'cbi':  // class by id: p[1]=id p[2]=value
			document.getElementById(p[1]).setAttribute("class",p[2]);
			break;
		case 'ibi':  // innerHTML by id: p[1]=id p[2]=value
			var n=(p.length-1)/2;
			for(i=0;i<n;i++){
				var idn=p[(i*2)+1];
				d=document.getElementById(idn);
				if(d!==null) {
					var value=p[(i*2)+2];
					if(d.tagName=="INPUT") d.value=value;
					else d.innerHTML=value;
					// a better way?
					if(idn=="heap")	gh.plot(value);
					else if(idn=="Q") gq.plot(value);
					else if(idn=="pps")	gp.plot(value);
					else if(idn=="adc")	ga.plot(value);									
					else if(idn=="load") gl.plot(value);									
				}
			}
			break;
		case 'clr':
			var clr=document.getElementById(p[1]);
			while (clr.firstChild) clr.removeChild(clr.firstChild);
			break;
		case 'rfs':
			var i=document.getElementById("iot");
			var last=document.getElementById("last");
			var v1=p[1];
			var v2=p[2];
			var divlhs=document.createElement("div");
			divlhs.className="lhs";
			divlhs.innerHTML=v1;
			i.insertBefore(divlhs,last);			
			var divled=document.createElement("div");
			divled.className="output ld led-"+(v2=="0" ? "red":"green");
			divled.id=v1;
			divled.onclick=rfClick;
			var divrhs=document.createElement("div");
			divrhs.className="rhs";										
			divrhs.appendChild(divled);
			i.insertBefore(divrhs,last);
			break;
		case 'sss':
			for(var i=0;i<chx.length;i++) chx[i].checked = (p[i+2]=="1") ? true:false;
			break;
	}	
}
