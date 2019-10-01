type suggestion = {
  image: string,
  username: string,
};

let brainwaltzera = {
  image: "https://i1.sndcdn.com/avatars-000327108074-pngztm-t500x500.jpg",
  username: "brainwaltzera",
};

let truants = {
  image: "https://i1.sndcdn.com/avatars-000216196785-lytqg7-t500x500.jpg",
  username: "truants",
};

let all = [truants, brainwaltzera];

let getStyle = image => {
  ReactDOMRe.Style.make(~background="url(" ++ image ++ ") center", ());
};

[@react.component]
let make = (~setUsername) => {
  <div className="pa3 white b">
    <p className="f3"> {React.string("Maybe start with...")} </p>
    <div className="flex">
      {React.array(
         Belt.List.map(all, item =>
           <button
             key={item.username}
             className="bg-transparent outline-0 bn tl white b pointer"
             onClick={_ => setUsername(item.username)}>
             <div className="w4 w5-l grow">
               <div className="aspect-ratio aspect-ratio--1x1">
                 <div
                   className="aspect-ratio--object cover"
                   style={getStyle(item.image)}
                 />
               </div>
               <span> {React.string(item.username)} </span>
             </div>
           </button>
         )
         |> Array.of_list,
       )}
    </div>
  </div>;
};