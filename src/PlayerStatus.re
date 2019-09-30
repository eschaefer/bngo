[@bs.module "./images/play.svg"] external play: string = "default";
[@bs.module "./images/pause.svg"] external pause: string = "default";

[@react.component]
let make = (~isPlaying, ~handleClick) => {
  <button className="bn outline-0 pointer bg-transparent" onClick=handleClick>
    {isPlaying
       ? <img className="w-33" src=pause alt="Pause" />
       : <img className="w-33" src=play alt="Play" />}
  </button>;
};